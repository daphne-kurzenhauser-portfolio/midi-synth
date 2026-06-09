#ifndef MIDI_H
#define MIDI_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include "note_lut.h"
#include "midi_lut.h"
#include "notes.h"

#define MAX_MIDI_CHANNELS   4

typedef enum {
  ChanNoteOff,
  ChanNoteOn,
  ChanKeyPressure,
  ChanControlChange,
  ChanChannelMode,
  ChanProgramChange,
  ChanChannelPressure,
  ChanPitchBend,
  SysCommonMsg,
  SysRealtimeMsg,
  SysExclusiveMsg
} msgType;

typedef struct MidiCommonData {
  u8 flags;
} MidiCommonData;

typedef struct MidiMsg {
  msgType msg_type;           ///< message type
  unsigned char status_byte;
  unsigned char data_buf[2];             ///< buffer of data bytes
  unsigned char num_data_bytes;
  MidiCommonData *mididata;
} MidiMsg;

typedef struct MidiController {
  MidiCommonData mididata;      ///< struct to hold and pass common data 
  MidiMsg midi_buffer[64];      ///< buffer of MidiMsg objects
  u8 buf_iter_idx;
  snd_rawmidi_t *input_handler; ///< input handler for raw MIDI data
  char *devname;                ///< named device node
} MidiController;

int parseMidiBuffer(MidiController *midi_ctl, unsigned char *buf);
int dispatchCurrentBuffer(MidiController *midi_ctl);
void dispatchNoteOn(MidiMsg* msg);
void dispatchNoteOff(MidiMsg* msg);
void dispatchKeyPressure(MidiMsg* msg);
void dispatchControlChange(MidiMsg* msg);
void dispatchChannelMode(MidiMsg* msg);
void dispatchProgramChange(MidiMsg* msg);
void dispatchChannelPressure(MidiMsg* msg);
void dispatchPitchBend(MidiMsg* msg);

void print_midi_msg(unsigned char *buf);
const char* fmt_status_byte(unsigned char buf);

#endif
