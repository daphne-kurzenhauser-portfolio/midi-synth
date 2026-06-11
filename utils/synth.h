#ifndef SYNTH_H
#define SYNTH_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "midi.h"
#include "note_lut.h"
#include "playback.h"

#define HALF_AMPLITUDE      16384.0f
#define QUARTER_AMPLITUDE   8192.0f
#define EIGHTH_AMPLITUDE    4096.0f

typedef enum { SawtoothWave, SineWave, SquareWave, TriangleWave } Wave;

/// Structs for sharing common and initializer data
typedef struct synthInitData {
  Wave wave_type;
  u32 env_attack_ms;
  u32 env_release_ms;
  u8 flags;
} synthInitData;

typedef struct synthCommonData {
  Wave wave_type;
  u32 env_attack_buf;
  u32 env_release_buf;
  s16 max_amplitude;
  u8 flags;
} synthCommonData;


///
typedef struct synthNote {
  Note midi_note;
  double freq;
  s16 peak_amplitude;
  u32 attack_buf_rem;
  u32 release_buf_rem;
  int isOn;
} synthNote;

typedef struct SynthController {
  MidiController midi_ctl;
  PlaybackController playback_ctl;
  synthNote notes[121];
  synthCommonData synth_cdata;
} SynthController;

int init_synth_controller(SynthController *synth_ctl, synthInitData init_data);

int dispatch_current_buffer(SynthController *synth_ctl);
void dispatch_note_on(SynthController *synth_ctl, MidiMsg* msg);
void dispatch_note_off(SynthController *synth_ctl, MidiMsg* msg);
//void dispatch_key_pressure(MidiMsg* msg);
//void dispatch_control_change(MidiMsg* msg);
//void dispatch_channel_mode(MidiMsg* msg);
//void dispatch_program_change(MidiMsg* msg);
//void dispatch_channel_pressure(MidiMsg* msg);
//void dispatch_pitch_bend(MidiMsg* msg);

u32 build_sample_buf(SynthController *synth_ctl);
s16 new_note_sample(synthNote* note, double t, synthCommonData* cdata);
s16 new_sample_sawtooth(synthNote* note, double t, double amplitude_factor);
//s16 new_sample_sine(synthNote* note, double t, double amplitude_factor);
//s16 new_sample_square(synthNote* note, double t, double amplitude_factor);
//s16 new_sample_triangle(synthNote* note, double t, double amplitude_factor);

#endif
