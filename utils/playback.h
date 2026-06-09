#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "midi.h"
#include "note_lut.h"

#define SAMPLE_RATE         48000
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
  synthNote notes[121];
  synthCommonData synth_cdata;
} synthController;

int initSynthController(SynthController *synth_ctl, synthInitData init_data);

int dispatchCurrentBuffer(SynthController *synth_ctl);
void dispatchNoteOn(SynthController *synth_ctl, MidiMsg* msg);
void dispatchNoteOff(SynthController *synth_ctl, MidiMsg* msg);
//void dispatchKeyPressure(MidiMsg* msg);
//void dispatchControlChange(MidiMsg* msg);
//void dispatchChannelMode(MidiMsg* msg);
//void dispatchProgramChange(MidiMsg* msg);
//void dispatchChannelPressure(MidiMsg* msg);
//void dispatchPitchBend(MidiMsg* msg);

s16 playNote(synthNote* note, double t, synthCommonData* cdata);
s16 playSawtooth(synthNote* note, double t, double amplitude_factor);
//s16 playSine(synthNote* note, double t, double amplitude_factor);
//s16 playSquare(synthNote* note, double t, double amplitude_factor);
//s16 playTriangle(synthNote* note, double t, double amplitude_factor);

#endif
