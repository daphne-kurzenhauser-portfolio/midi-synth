#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include "utils/midi.h"

static snd_seq_t *seq;
static snd_seq_addr_t *ports;

static void usage(void)
{
  fprintf(stderr, "usage: ./synth [options]\n");
  fprintf(stderr, "  options:\n");
  fprintf(stderr, "  --help               : prints this message\n");
}

static void checkerr(const char *fn, int err)
{
  if (err < 0) {
    fprintf(stderr, "%s failed\n", fn);
    exit(0);
  }
}

int main(int argc,char** argv)
{
  int err;
  MidiController midi_ctl;

  for (int i=1; i<argc; i++) {
    const char *arg = argv[i];
    if (strcmp(arg, "--help") == 0) {
      usage();
      exit(0);
    }
  }
  /*
     struct pollfd *pfds;
     int npfds;
     ports = realloc(ports, sizeof(snd_seq_addr_t));
     err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
     checkerr("snd_seq_open", err);

     err = snd_seq_parse_address(seq, &ports[0], "20:0");
     checkerr("snd_seq_parse_address", err);

     err = snd_seq_create_simple_port(seq, "synth",
     SND_SEQ_PORT_CAP_WRITE | 
     SND_SEQ_PORT_CAP_SUBS_WRITE,
     SND_SEQ_PORT_TYPE_MIDI_GENERIC |
     SND_SEQ_PORT_TYPE_APPLICATION);
     checkerr("snd_seq_create_simple_port", err);

     err = snd_seq_connect_from(seq, 0, ports[0].client, ports[0].port);
     checkerr("snd_seq_connect_from", err);

     err = snd_seq_nonblock(seq, 1);
     checkerr("snd_seq_nonblock", err);

     printf("Sequencer connected!\n");

     npfds = snd_seq_poll_descriptors_count(seq, POLLIN);
     pfds = alloca(sizeof(*pfds) * npfds);
     for (;;) {
     snd_seq_poll_descriptors(seq, pfds, npfds, POLLIN);
     if (poll(pfds, npfds, -1) < 0)
     break;
     for (;;) {
     snd_seq_event_t *event;
     err = snd_seq_event_input(seq, &event);
     if (err < 0)
     break;
     if (event) {
     switch (event->type) {
     case SND_SEQ_EVENT_NOTEON:
     case SND_SEQ_EVENT_NOTEOFF:
     printf("Channel: %d, note %d, velocity %d\n", event->data.note.channel,
     event->data.note.note, event->data.note.velocity);
     case SND_SEQ_EVENT_KEYPRESS:
     printf("Aftertouch: %d, note %d, velocity %d\n", event->data.note.channel,
     event->data.note.note, event->data.note.velocity);
     default:
     printf("Unsupported event: %d\n", event->type);
     }
     }
     }
     fflush(stdout);
     }

     return 0;*/

  char *device_in = "default";
  snd_rawmidi_t *handle_in = 0;
  clockid_t cid = CLOCK_REALTIME;
  if (device_in) {
    err = snd_rawmidi_open(&handle_in,NULL,"hw:1,0,0",SND_RAWMIDI_NONBLOCK);
    if (err) {
      fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_in,err);
    }
    if (handle_in) {
      snd_rawmidi_read(handle_in, NULL, 0);
    }
  }

  if (handle_in) {
    int read =0;
    int npfds;
    struct pollfd *pfds;

    npfds = 1 + snd_rawmidi_poll_descriptors_count(handle_in);
    pfds = alloca(npfds * sizeof(struct pollfd));
    pfds[0].fd = -1;

    snd_rawmidi_poll_descriptors(handle_in, &pfds[1], npfds - 1);

    for (;;) {
      unsigned char buf[256];
      int i, length;
      unsigned short revents;
      struct timespec ts;

      err = poll(pfds, npfds, -1);
      if (err < 0) {
        fprintf(stderr, "poll failed: %s", strerror(errno));
        break;
      }

      if (clock_gettime(cid, &ts) < 0) {
        fprintf(stderr, "clock_getres (%d) failed: %s", cid, strerror(errno));
        break;
      }

      err = snd_rawmidi_poll_descriptors_revents(handle_in, &pfds[1], npfds - 1, &revents);
      if (err < 0) {
        fprintf(stderr, "cannot get poll events: %s", snd_strerror(errno));
        break;
      }
      if (revents & (POLLERR | POLLHUP))
        break;
      if (!(revents & POLLIN)) {
        if (pfds[0].revents & POLLIN)
          break;
        continue;
      }

      err = snd_rawmidi_read(handle_in, buf, sizeof(buf));
      if (err == -EAGAIN)
        continue;
      if (err < 0) {
        fprintf(stderr, "read error: %d - %s\n", (int)err, snd_strerror(err));
        break;
      }
      length = 0;
      for (i = 0; i < err; i++) {
        if (buf[i] != MIDI_CMD_COMMON_CLOCK && buf[i] != MIDI_CMD_COMMON_SENSING)
          buf[length++] = buf[i];
        if (length == 0)
          continue;
        read += length;

        err = parseMidiBuffer(&midi_ctl, buf);
        print_midi_msg(buf);
        fflush(stdout);
      }
    }
  }
  else {
    fprintf(stderr, "Handle doesn't exist\n");
  }

  if (handle_in) {
    snd_rawmidi_drain(handle_in);
    snd_rawmidi_close(handle_in);
  }

  return 0;
}
