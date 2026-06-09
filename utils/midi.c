#include "midi.h"

int parseMidiBuffer(MidiController *midi_ctl, unsigned char *buf) {
  midi_ctl->buf_iter_idx = (midi_ctl->buf_iter_idx + 1) % 64;
  MidiMsg *midi_msg = &(midi_ctl->midi_buffer[midi_ctl->buf_iter_idx]);
  midi_msg->status_byte = buf[0];

  switch (midi_msg->status_byte & STATUS_BYTE_MASK)
  {
    case STATUS_BYTE_NOTE_OFF:
      midi_msg->msg_type = ChanNoteOff;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      break;
    case STATUS_BYTE_NOTE_ON:
      midi_msg->msg_type = ChanNoteOn;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      break;
    case STATUS_BYTE_KEY_AFTERTOUCH:
      midi_msg->msg_type = ChanKeyPressure;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      break;
    case STATUS_BYTE_CONTROL_CHANGE:
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      if (midi_msg->data_buf[0] < 120) {
        midi_msg->msg_type = ChanControlChange;
      } else if (midi_msg->data_buf[0] <= 127) {
        midi_msg->msg_type = ChanChannelMode;
      } else {
        fprintf(stderr, "Data byte payload invalid\n");
        return -1;
      }
      break;
    case STATUS_BYTE_PROGRAM_CHANGE:
      midi_msg->msg_type = ChanProgramChange;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->num_data_bytes = 1;
      break;
    case STATUS_BYTE_CHANNEL_AFTERTOUCH:
      midi_msg->msg_type = ChanChannelPressure;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->num_data_bytes = 1;
      break;
    case STATUS_BYTE_PITCH_BEND:
      midi_msg->msg_type = ChanPitchBend;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      break;
  // F0H -> system message
  //  1111 0000 -> sys exclusive, arbitrary length
  //  1111 0xxx -> sys common, 0 to 2 data bytes
  //  1111 1xxx -> sys real time, 0 data byte
    case STATUS_BYTE_SYSTEM_MESSAGE:
      printf("System message received\n");
      break;
    default:
      return -1;
  }

  return 1;
}

int dispatchCurrentBuffer(MidiController *midi_ctl)
{
  MidiMsg *cur_msg = &(midi_ctl->midi_buffer[midi_ctl->buf_iter_idx]);
  switch (cur_msg->msg_type) {
    case ChanNoteOff:
      dispatchNoteOff(cur_msg);
    case ChanNoteOn:
      dispatchNoteOn(cur_msg);
    case ChanKeyPressure:
      //dispatchKeyPressure(cur_msg);
    case ChanControlChange:
      //dispatchControlChange(cur_msg);
    case ChanChannelMode:
      //dispatchChannelMode(cur_msg);
    case ChanProgramChange:
      //dispatchProgramChange(cur_msg);
    case ChanChannelPressure:
      //dispatchChannelPressure(cur_msg);
    case ChanPitchBend:
      //dispatchPitchBend(cur_msg);
    default:
      return 0;
  }
}

void dispatchNoteOn(MidiMsg* msg)
{
  Note midi_note = msg->data_buf[0];
  u8 velocity = msg->data_buf[1];
}

void dispatchNoteOff(MidiMsg* msg)
{
  Note midi_note = msg->data_buf[0];
  u8 velocity = msg->data_buf[1];
}

/*void dispatchKeyPressure(MidiMsg* msg)
{
  Note midi_note = msg->data_buf[0];
  u8 pressure = msg->data_buf[1];
}

void dispatchControlChange(MidiMsg* msg)
{
}

void dispatchChannelMode(MidiMsg* msg)
{
}

void dispatchProgramChange(MidiMsg* msg)
{
}

void dispatchChannelPressure(MidiMsg* msg)
{
}

void dispatchPitchBend(MidiMsg* msg)
{
}*/

void print_midi_msg(unsigned char *buf)
{
  char status[32];
  switch (buf[0]) {
    case STATUS_BYTE_NOTE_OFF:
      snprintf(status, sizeof(status), "%s", "NOTE_OFF");
      break;
    case 0x90:
      snprintf(status, sizeof(status), "%s", "NOTE_ON");
      break;
    case STATUS_BYTE_KEY_AFTERTOUCH:
      snprintf(status, sizeof(status), "%s", "KEY_AFTERTOUCH");
      break;
    case STATUS_BYTE_CONTROL_CHANGE:
      snprintf(status, sizeof(status), "%s", "CONTROL_CHANGE");
      break;
    case STATUS_BYTE_PROGRAM_CHANGE:
      snprintf(status, sizeof(status), "%s", "PROGRAM_CHANGE");
      break;
    default:
      snprintf(status, sizeof(status), "%s", "DEFAULT");
      break;
  }
  printf("Status Byte: %s\n", status);
}
