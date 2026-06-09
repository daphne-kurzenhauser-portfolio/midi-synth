cmake_minimum_required(VERSION 3.10)

project(synth)

list(APPEND CMAKE_MODULE_PATH)

find_package(portaudio REQUIRED)

set(SYNTH_SRC 
  utils/midi.c 
  #utils/music_utils.c
  #utils/wav_utils.c
)
set(SYNTH_HEADERS 
  utils/midi.h 
  utils/midi_lut.h 
  #utils/music_utils.h 
  #utils/music_tbls.h
  #utils/wav_utils.h
)

add_library(synthlib STATIC ${SYNTH_HEADERS} ${SYNTH_SRC})

add_executable(${PROJECT_NAME} main.c ${SYNTH_HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE synthlib portaudio)
