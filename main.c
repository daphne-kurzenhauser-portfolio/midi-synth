#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include "utils/midi.h"

static void usage(void)
{
  fprintf(stderr, "usage: ./synth [options]\n");
  fprintf(stderr, "  options:\n");
  fprintf(stderr, "  --help               : prints this message\n");
}

static void checkerr(const char *fn, int err)
{
  if (err < 0) {
    fprintf(stderr, "%s failed\n", fn);
    exit(0);
  }
}

int main(int argc,char** argv)
{
  int err;
  int do_device = 0;
  SynthController synth_ctl;
  synthInitData synth_init_data = {
    .wave_type = SawtoothWave,
    .env_attack_ms = 10,
    .env_release_ms = 10,
    .max_amplitude = 16384,
    .flags = 0
  };

  MidiController *midi_ctl = &(synth_ctl.midi_ctl);
  midiInitData midi_init_data = {
    .devname = "default",
    .flags = 0
  };
  snd_rawmidi_t *midi_in = midi_ctl->hdl;
  clockid_t cid = CLOCK_REALTIME;

  for (int i=1; i<argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--help") == 0) {
      usage();
      exit(0);
    } else if (strcmp(arg, "--device") == 0) {
      if ((i+1) == argc) {
        fprintf(stderr, "must supply device handle for --device flag\n");
        exit(0);
      }
      strncpy(midi_init_data.devname, argv[i+1], sizeof(midi_init_data.devname));
      printf("Using device at MIDI: %s\n", midi_init_data.devname);
      do_device = 1;
    }
  }

  // INITIALIZE
  err = init_synth_controller(&synth_ctl, synth_init_data);

  err = init_midi_controller(midi_ctl, midi_init_data);
  if (err < 0) {
    fprintf(stderr, "Failed to initialize MidiController struct. Exiting\n");
    return -1;
  }

  for (;;) {
    unsigned char buf[256];
    int i, length;
    unsigned short revents;
    struct timespec ts;

    err = poll(midi_ctl->pfds, midi_ctl->npfds, -1);
    checkerr("poll", err);

    /*if (clock_gettime(cid, &ts) < 0) {
      fprintf(stderr, "clock_getres (%d) failed: %s", cid, strerror(errno));
      break;
    }

    err = snd_rawmidi_poll_descriptors_revents(midi_in, &pfds[1], npfds - 1, &revents);
    checkerr("snd_rawmidi_poll_descriptors_revents", err);
    if (revents & (POLLERR | POLLHUP))
      break;

    if (!(revents & POLLIN)) {
      if (pfds[0].revents & POLLIN)
        break;
      continue;
    }*/

    err = snd_rawmidi_read(midi_in, buf, sizeof(buf));
    if (err == -EAGAIN)
      continue;
    if (err < 0) {
      fprintf(stderr, "read error: %d - %s\n", (int)err, snd_strerror(err));
      break;
    }

    length = 0;
    for (i = 0; i < err; i++) {
      if (buf[i] != MIDI_CMD_COMMON_CLOCK && buf[i] != MIDI_CMD_COMMON_SENSING)
        buf[length++] = buf[i];
      if (length == 0) { continue; }
    }

    err = parse_midi_buffer(midi_ctl, buf);
    if (err < 1) { 
      continue; 
    } else {
      err = dispatch_current_buffer(&synth_ctl);
    }

    fflush(stdout);
  }

  if (midi_in) {
    snd_rawmidi_drain(midi_in);
    snd_rawmidi_close(midi_in);
  }

  return 0;
}
