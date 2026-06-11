#include "midi.h"

int init_midi_controller(MidiController *midi_ctl, midiInitData init_data)
{
  midi_ctl->midi_data.flags = init_data.flags;
  int err = snd_rawmidi_open(&midi_ctl->hdl,NULL,init_data.devname,SND_RAWMIDI_NONBLOCK);
  if (err < 0) {
    fprintf(stderr, "Failed to open rawmidi\n");
    return -1;
  }

  if (midi_ctl->hdl) {
    snd_rawmidi_read(midi_ctl->hdl, NULL, 0);
    fprintf(stdout, "MidiController initialized\n");
    return 1;
  }
  else {
    fprintf(stderr, "Handle doesn't exist\n");
    return -1;
  }
}

int fetch_midi_message(MidiController *midi_ctl, unsigned char *buf)
{
  int length, err;

  err = snd_rawmidi_read(midi_ctl->hdl, buf, sizeof(buf));
  if (err == -EAGAIN)
    return 0;
  if (err < 0) {
    fprintf(stderr, "read error: %d - %s\n", (int)err, snd_strerror(err));
    return -1;
  }

  length = 0;
  for (int i = 0; i < err; i++) {
    if (buf[i] != MIDI_CMD_COMMON_CLOCK && buf[i] != MIDI_CMD_COMMON_SENSING)
      buf[length++] = buf[i];
  }
  return 1;
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
      return 0;
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
  fprintf(stdout, "Status Byte: %s\n", status);
}
