#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "midi_lut.h"
#include "note_lut.h"
#include "synth.h"

const double hz_lut[121] = {  
  1.0,  1.0,   1.0,  1.0,   1.0,  1.0,  1.0,   1.0,  1.0,   1.0,  1.0,   1.0,
  C0_Hz,Cs0_Hz,D0_Hz,Ds0_Hz,E0_Hz,F0_Hz,Fs0_Hz,G0_Hz,Gs0_Hz,A0_Hz,As0_Hz,B0_Hz,
  C1_Hz,Cs1_Hz,D1_Hz,Ds1_Hz,E1_Hz,F1_Hz,Fs1_Hz,G1_Hz,Gs1_Hz,A1_Hz,As1_Hz,B1_Hz,
  C2_Hz,Cs2_Hz,D2_Hz,Ds2_Hz,E2_Hz,F2_Hz,Fs2_Hz,G2_Hz,Gs2_Hz,A2_Hz,As2_Hz,B2_Hz,
  C3_Hz,Cs3_Hz,D3_Hz,Ds3_Hz,E3_Hz,F3_Hz,Fs3_Hz,G3_Hz,Gs3_Hz,A3_Hz,As3_Hz,B3_Hz,
  C4_Hz,Cs4_Hz,D4_Hz,Ds4_Hz,E4_Hz,F4_Hz,Fs4_Hz,G4_Hz,Gs4_Hz,A4_Hz,As4_Hz,B4_Hz,
  C5_Hz,Cs5_Hz,D5_Hz,Ds5_Hz,E5_Hz,F5_Hz,Fs5_Hz,G5_Hz,Gs5_Hz,A5_Hz,As5_Hz,B5_Hz,
  C6_Hz,Cs6_Hz,D6_Hz,Ds6_Hz,E6_Hz,F6_Hz,Fs6_Hz,G6_Hz,Gs6_Hz,A6_Hz,As6_Hz,B6_Hz,
  C7_Hz,Cs7_Hz,D7_Hz,Ds7_Hz,E7_Hz,F7_Hz,Fs7_Hz,G7_Hz,Gs7_Hz,A7_Hz,As7_Hz,B7_Hz,
  C8_Hz,Cs8_Hz,D8_Hz,Ds8_Hz,E8_Hz,F8_Hz,Fs8_Hz,G8_Hz,Gs8_Hz,A8_Hz,As8_Hz,B8_Hz,
  C9_Hz
};

int init_synth_controller(SynthController *synth_ctl, synthInitData init_data)
{
  synth_ctl->synth_cdata.env_attack_buf = SAMPLE_RATE * init_data.env_attack_ms / 1000;
  synth_ctl->synth_cdata.env_release_buf = SAMPLE_RATE * init_data.env_release_ms / 1000;
  synth_ctl->synth_cdata.max_amplitude = HALF_AMPLITUDE;
  synth_ctl->synth_cdata.wave_type = init_data.wave_type;

  for (int i=0; i<121; i++) {
    synth_ctl->notes[i].midi_note = i;
    synth_ctl->notes[i].freq = hz_lut[i];
    synth_ctl->notes[i].peak_amplitude = 0;
    synth_ctl->notes[i].attack_buf_rem = 0;
    synth_ctl->notes[i].release_buf_rem = 0;
    synth_ctl->notes[i].isOn = 0;
  }

  fprintf(stdout, "SynthController initialized\n");
  return 1;
}

int dispatch_current_buffer(SynthController *synth_ctl)
{
  MidiController *this_midi_ctl = &(synth_ctl->midi_ctl);
  MidiMsg *cur_msg = &(this_midi_ctl->midi_buffer[this_midi_ctl->buf_iter_idx]);
  switch (cur_msg->msg_type) {
    case ChanNoteOff:
      dispatch_note_off(synth_ctl, cur_msg);
      break;
    case ChanNoteOn:
      dispatch_note_on(synth_ctl, cur_msg);
      break;
    default:
      return 0;
  }
  return 1;
}

void dispatch_note_on(SynthController *synth_ctl, MidiMsg* msg)
{
  synthNote* note = &(synth_ctl->notes[msg->data_buf[0]]);
  u8 velocity = msg->data_buf[1];

  if (velocity > 0) {
    if (note->isOn) {
      //fprintf(stdout, "Note already on, ignoring\n");
      return;
    }
    note->peak_amplitude = synth_ctl->synth_cdata.max_amplitude * pow((velocity/127.0), 2.0);
    note->attack_buf_rem = synth_ctl->synth_cdata.env_attack_buf;
    note->release_buf_rem = 0;
    note->isOn = 1;
    fprintf(stdout, "NOTE ON: peak amplitude %d, attack %d\n", note->peak_amplitude, note->attack_buf_rem);
  } else { // don't play note
    note->release_buf_rem = synth_ctl->synth_cdata.env_release_buf;
  }
}

void dispatch_note_off(SynthController *synth_ctl, MidiMsg* msg)
{
  synthNote* note = &(synth_ctl->notes[msg->data_buf[0]]);
  u8 velocity = msg->data_buf[1];

  if (!note->isOn) {
    //fprintf(stdout, "Note already off, ignoring\n");
    return;
  }
  note->release_buf_rem = synth_ctl->synth_cdata.env_release_buf;
  fprintf(stdout, "NOTE OFF: release %d\n", note->release_buf_rem);
}

u32 build_sample_buf(SynthController *synth_ctl)
{
  double t;
  PlaybackController *player = &(synth_ctl->playback_ctl);
  for (int i=0; i<SAMPLES_PER_BUF; i++) {
    player->playback_buf[i] = 0;
    t = (double)player->phase / SAMPLE_RATE;
    for (int midi_note=0; midi_note<121; midi_note++) {
      synthNote* note = &(synth_ctl->notes[midi_note]);
      if (note->isOn) {
        player->playback_buf[i] += new_note_sample(note, t, &(synth_ctl->synth_cdata));
      }
    }
    player->phase++;
    //printf("%d ", player->playback_buf[i]);
  }
  write_frame(player);
  fflush(stdout);
  return SAMPLES_PER_BUF;
}

/// Get the s16 amplitude value for a note at a given time domain value t
s16 new_note_sample(synthNote* note, double t, synthCommonData* cdata)
{
  /// We need to consider envelope regardless of waveform, so do that first
  double amplitude_factor;
  if (note->attack_buf_rem && cdata->env_attack_buf) {
    amplitude_factor = (cdata->env_attack_buf - note->attack_buf_rem) / cdata->env_attack_buf;
    note->attack_buf_rem--;
  } else if (note->release_buf_rem && cdata->env_release_buf) {
    amplitude_factor = 1.0 - ((cdata->env_release_buf - note->release_buf_rem) / cdata->env_release_buf);
    note->release_buf_rem--;
    if (note->release_buf_rem == 0) {
      /// if we have no remaining release buffers, that means this is our final sample
      note->isOn = 0;
    }
  } else {
    amplitude_factor = 1.0;
  }
  amplitude_factor = amplitude_factor * note->peak_amplitude;

  /// 
  switch (cdata->wave_type) {
    case SawtoothWave:
      return new_sample_sawtooth(note, t, amplitude_factor);
    default:
      return 0;
  }

  return 0;
}

/// Get the s16 amplitude value for a sawtooth wave at a given time domain value t
s16 new_sample_sawtooth(synthNote* note, double t, double amplitude_factor)
{
  double freq = 1 / (note->freq);
  double amplitude_f = 2 * (t/freq - floor(0.5 + t/freq));
  return (s16)(amplitude_factor * amplitude_f);
}
