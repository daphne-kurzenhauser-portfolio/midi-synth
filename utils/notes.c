#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "notes.h"
#include "note_lut.h"
#include "midi_lut.h"

const float hz_lut[NUM_NOTES] = {  
0.0,  0.0,   0.0,  0.0,   0.0,  0.0,  0.0,   0.0,  0.0,   0.0,  0.0,   0.0,
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
