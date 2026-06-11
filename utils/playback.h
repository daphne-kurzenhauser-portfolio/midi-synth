#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "portaudio.h"
#include "note_lut.h"
#include "midi_lut.h"

#define SAMPLE_RATE         48000
#define LATENCY_MS          10
#define SAMPLES_PER_BUF     (SAMPLE_RATE * LATENCY_MS / 1000)

typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint32_t u32;

typedef struct PlaybackController {
  PaStream *stream;
  PaStreamParameters input_params;
  PaStreamParameters output_params;
  s16 playback_buf[SAMPLES_PER_BUF];
  u32 phase;
} PlaybackController;

int init_playback_controller(PlaybackController* playback_ctl);
int write_frame(PlaybackController* playback_ctl);
int end_playback(PlaybackController* playback_ctl);

#endif
