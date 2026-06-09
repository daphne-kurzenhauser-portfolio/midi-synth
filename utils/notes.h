#ifndef NOTES_H
#define NOTES_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "note_lut.h"
#include "midi_lut.h"

typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint32_t u32;

typedef struct synthNote {
  double freq;
  s16 volume;
  u8 flags;
} synthNote;

u32 playSawtooth(synthNote* note);
u32 playSine(synthNote* note);
u32 playSquare(synthNote* note);
u32 playTriangle(synthNote* note);

#endif
