#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "playback.h"

int init_playback_controller(PlaybackController* playback_ctl)
{
  PaError err = Pa_Initialize();
  if (err != paNoError) goto error;

  playback_ctl->phase = 0;

  playback_ctl->input_params.channelCount = 1;
  playback_ctl->input_params.sampleFormat = paInt16;

  playback_ctl->output_params.device = Pa_GetDefaultOutputDevice();
  playback_ctl->output_params.channelCount = 1;
  playback_ctl->output_params.sampleFormat = paInt16;
  playback_ctl->output_params.suggestedLatency = Pa_GetDeviceInfo(playback_ctl->output_params.device)->defaultHighOutputLatency;

  err = Pa_OpenStream(&playback_ctl->stream,
      &playback_ctl->input_params, &playback_ctl->output_params,
      SAMPLE_RATE, SAMPLES_PER_BUF,
      paClipOff,
      NULL, NULL);
  if (err != paNoError) goto error;

  err = Pa_StartStream(playback_ctl->stream);
  if (err != paNoError) goto error;

  fprintf(stdout, "Ready to stream!\n");
  return 1;

error:
  fprintf(stderr, "Error occured with PortAudio: %s\n", Pa_GetErrorText(err));
  Pa_Terminate();
  return -1;
}

int write_frame(PlaybackController* playback_ctl)
{
  Pa_WriteStream(playback_ctl->stream, &playback_ctl->playback_buf[0], SAMPLES_PER_BUF);
  return 1;
}

int end_playback(PlaybackController* playback_ctl)
{
  PaError err = Pa_StopStream(playback_ctl->stream);
  if (err != paNoError) goto error;
  err = Pa_CloseStream(playback_ctl->stream);
  if (err != paNoError) goto error;

  Pa_Terminate();
  return 1;

error:
  fprintf(stderr, "Error occured with PortAudio: %s\n", Pa_GetErrorText(err));
  Pa_Terminate();
  return -1;
}
