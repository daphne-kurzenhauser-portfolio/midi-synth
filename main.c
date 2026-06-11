#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include "portaudio.h"
#include "utils/midi.h"
#include "utils/synth.h"

static volatile sig_atomic_t stop = 0;

static void usage(void)
{
  fprintf(stderr, "usage: ./synth [options]\n");
  fprintf(stderr, "  options:\n");
  fprintf(stderr, "  --help               : prints this message\n");
  fprintf(stderr, "  --midi-device DEVICE : specifies device for raw MIDI input\n");
  fprintf(stderr, "               example : ./synth --midi-device hw:1,0,0\n");
}

static void checkerr(const char *fn, int err)
{
  if (err < 0) {
    fprintf(stderr, "%s failed\n", fn);
    exit(0);
  }
}

static void sighandler(int sig ATTRIBUTE_UNUSED)
{
  stop = 1;
}

typedef struct {
  u32 phase;
  synthNote (*notes)[121];
  synthCommonData* synth_cdata;
} callbackData;

static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      const PaStreamCallbackFlags statusFlags,
                      void *userData)
{
  callbackData *data = (callbackData*)userData;
  s16 *out = (s16*)outputBuffer;
  double t;

  for (int i=0; i<framesPerBuffer; i++) {
    out[i] = 0;
    t = (double)data->phase / SAMPLE_RATE;
    for (int midi_note=0; midi_note<121; midi_note++) {
      synthNote* note = &((*data->notes)[midi_note]);
      if (note->isOn) {
        out[i] += new_note_sample(note, t, data->synth_cdata);
      }
    }
    data->phase++;
  }
  return 0;
}

static callbackData data;
int main(int argc,char** argv)
{
  int err;
  int do_device = 0;
  SynthController synth_ctl;
  synthInitData synth_init_data = {
    .wave_type = SawtoothWave,
    .env_attack_ms = 10,
    .env_release_ms = 10,
    .flags = 0
  };

  MidiController *midi_ctl = &(synth_ctl.midi_ctl);
  midiInitData midi_init_data = {
    .devname = "default",
    .flags = 0
  };
  //clockid_t cid = CLOCK_REALTIME;

  for (int i=1; i<argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--help") == 0) {
      usage();
      exit(0);
    } else if (strcmp(arg, "--midi-device") == 0) {
      if ((i+1) == argc) {
        fprintf(stderr, "must supply device handle for --midi-device flag\n");
        exit(0);
      }
      strncpy(midi_init_data.devname, argv[i+1], sizeof(midi_init_data.devname));
      fprintf(stdout, "Using device at MIDI: %s\n", midi_init_data.devname);
      do_device = 1;
    }
  }

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  // INITIALIZE
  err = init_synth_controller(&synth_ctl, synth_init_data);

  err = init_midi_controller(midi_ctl, midi_init_data);
  if (err < 0) {
    fprintf(stderr, "Failed to initialize MidiController struct. Exiting\n");
    return -1;
  }

  PaStream *stream;
  data.notes = &(synth_ctl.notes);
  data.synth_cdata = &(synth_ctl.synth_cdata);
  PaError pa_err = Pa_Initialize();

  pa_err = Pa_OpenDefaultStream(&stream,
      0,
      1,
      paInt16,
      SAMPLE_RATE,
      SAMPLES_PER_BUF,
      paCallback,
      &data);
  if (pa_err != paNoError) goto error;

  pa_err = Pa_StartStream(stream);
  if (pa_err != paNoError) goto error;

  while (!stop) {
    unsigned char buf[64];
    err = fetch_midi_message(midi_ctl, buf);
    if (err < 0) {
      fprintf(stderr, "Fatal error occurred\n");
      break;
    } else if (err > 0) {
      err = parse_midi_buffer(midi_ctl, buf);
      if (err > 0) { 
        err = dispatch_current_buffer(&synth_ctl);
      }
    }
  }

  fprintf(stdout, "Left main loop\n");

  if (midi_ctl->hdl) {
    snd_rawmidi_drain(midi_ctl->hdl);
    snd_rawmidi_close(midi_ctl->hdl);
  }

  pa_err = Pa_StopStream(stream);
  if (pa_err != paNoError) goto error;
  pa_err = Pa_CloseStream(stream);
  if (pa_err != paNoError) goto error;
  Pa_Terminate();

  fprintf(stdout, "Exiting!\n");

  return 0;

error:
  fprintf(stderr, "Error occured with PortAudio: %s\n", Pa_GetErrorText(err));
  Pa_Terminate();
  return -1;
}
