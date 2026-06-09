#include "midi.h"

int init_midi_controller(MidiController *midi_ctl, midiInitData init_data)
{
  midi_ctl->midi_data.flags = init_data.flags;
  snd_rawmidi_t *midi_in = midi_ctl->hdl;

  err = snd_rawmidi_open(&midi_in,NULL,init_data.devname,SND_RAWMIDI_NONBLOCK);
  if (err < 0) {
    fprintf(stderr, "Failed to open rawmidi\n");
    return -1;
  }

  if (midi_in) {
    snd_rawmidi_read(midi_in, NULL, 0);

    midi_ctl->npfds = 1 + snd_rawmidi_poll_descriptors_count(midi_in);
    midi_ctl->pfds = alloca(midi_ctl->npfds * sizeof(struct pollfd));
    midit_ctl->pfds[0].fd = -1;

    snd_rawmidi_poll_descriptors(midi_in, &(midi_ctl->pfds[1]), midi_ctl->npfds - 1);
  }
  else {
    fprintf(stderr, "Handle doesn't exist\n");
    return -1;
  }
}

int parse_midi_buffer(MidiController *midi_ctl, unsigned char *buf) {
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
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
    case STATUS_BYTE_NOTE_ON:
      midi_msg->msg_type = ChanNoteOn;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
    /*case STATUS_BYTE_KEY_AFTERTOUCH:
      midi_msg->msg_type = ChanKeyPressure;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
    case STATUS_BYTE_CONTROL_CHANGE:
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
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
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
    case STATUS_BYTE_CHANNEL_AFTERTOUCH:
      midi_msg->msg_type = ChanChannelPressure;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->num_data_bytes = 1;
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
    case STATUS_BYTE_PITCH_BEND:
      midi_msg->msg_type = ChanPitchBend;
      midi_msg->data_buf[0] = buf[1];
      midi_msg->data_buf[1] = buf[2];
      midi_msg->num_data_bytes = 2;
      midi_msg->channel = (midi_msg->status_byte & STATUS_BYTE_CHANNEL_MASK);
      break;
  // F0H -> system message
  //  1111 0000 -> sys exclusive, arbitrary length
  //  1111 0xxx -> sys common, 0 to 2 data bytes
  //  1111 1xxx -> sys real time, 0 data byte
    case STATUS_BYTE_SYSTEM_MESSAGE:
      printf("System message received\n");
      return 0;*/
    default:
      return -1;
  }

  return 1;
}

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
