#ifndef MIDI_H
#define MIDI_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include "midi_lut.h"

#define MAX_MIDI_CHANNELS   4

typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint32_t u32;

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

typedef struct midiInitData {
  char devname[16];
  u8 flags;
} midiInitData;

typedef struct midiCommonData {
  u8 flags;
} midiCommonData;

typedef struct MidiMsg {
  msgType msg_type;           ///< message type
  unsigned char status_byte;
  unsigned char data_buf[2];             ///< buffer of data bytes
  int num_data_bytes;
  u8 channel; 
  midiCommonData *mididata;
} MidiMsg;

typedef struct MidiController {
  MidiMsg midi_buffer[64];      ///< buffer of MidiMsg objects
  midiCommonData midi_data;      ///< struct to hold and pass common data 
  u8 buf_iter_idx;
  snd_rawmidi_t *hdl;           ///< input handler for raw MIDI data
  struct pollfd *pfds;
  int npfds;
} MidiController;

int init_midi_controller(MidiController *midi_ctl, midiInitData init_data);

int fetch_midi_message(MidiController *midi_ctl);
int parse_midi_buffer(MidiController *midi_ctl, unsigned char *buf);

void print_midi_msg(unsigned char *buf);

#endif
