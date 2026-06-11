# MIDI Software Synthesizer

## Description

This repo provides a C-based, minimalist implementation for a MIDI software synthesizer on Linux.
ALSA is used to fetch the raw MIDI data from a provided device port, and PortAudio is used to
play the audio to system output--outside of that, all of the processing, parsing, and dispatching
of MIDI messages is done from scratch, as is the sound generation.

## Build

This application uses a cmake build. To build, run:
```
cd ~
mkdir -p build/midi-synth && cd build/midi-synth 
cmake <repo location>
make
```

The only dependency for building the software is [PortAudio](https://files.portaudio.com/download.html). 
Running the application will require having ALSA and PortAudio configured on your Linux system, however.

## Run

In order to run the software, all you need is to make sure you have a MIDI device plugged in and
provide the software with its name. Right now, the software isn't configured to have ALSA 
automatically find a default MIDI device, so you'll have to specify one manually with the 
`--midi-device` flag. Running `amidi -l` in the command line will show you your available devices--in 
my case, it was usually `'hw:1,0,0'`.

There are some additional flags provided for sound output:
```
"usage: ./aleatoric [options]
"  options:
"    --help                 : prints this message
"    --midi-device DEVICE   : specifies device for raw MIDI input
"                   example : ./synth --midi-device 'hw:1,0,0'
"    --sine                 : enables sine wave output for audio
"    --square               : enables square wave output for audio
```

## Notes

All notes are played with a fixed attack-release envelope with 10ms on both ends. The peak amplitude
for a given note is computed with a standard square-law curve conversion from the supplied MIDI velocity.

This synthesizer currently only supports a single MIDI channel, with no downstream discrimination between
any channels specified in the MIDI message data. It is, however, semi-polyphonic--each one of the possible
127 MIDI notes (including placeholders) has up to one voice, so several notes can be played at a time.
However, waves for all voices are simply added on top of each other, so there's no equalization done to
prevent clipping for a large amount of notes.

### Implementation Details

Two lookup table files are used for most of the macro mapping:
- `utils/midi_lut.h` defines macros for MIDI message status bytes and masks. Currently, these
are mostly all unimplemented, but allow for expansion per MIDI 1.0 specification.
- `utils/note_lut.h` defines macros and enums for MIDI notes and their corresponding
frequencies in Hz.

Software for utilities are split across two modules:
- `midi.c/midi.h` contain structs and functions for managing MIDI objects and parsing 
messages. Dispatching of MIDI messages is handled by the synth module; 
but this is where pulling in the raw data from ALSA is done. Some code for the MIDI 
module was based off of existing alsa-utils code.
- `synth.c/synth.h` contain structs and functions for dispatching MIDI messages and
calculating the amplitudes for individual sound samples.

The synth module implements the application of the envelope and the generation of sound 
sample values; however, the actual invocation of this is done through a PortAudio callback.

### Fixes and Improvements

