/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
  spice.github/server/test/test_playback.c
*/
#include "bd_test.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>

#include <spice.h>
//#include <spice-audio.h>
//#include "reds.h"

/* test the audio playback interface. Really basic no frils test - create
 * a single tone sinus sound (easy to hear clicks if it is generated badly
 * or is transmitted badly).
 *
 * TODO: Was going to do white noise to test compression too.
 *
 * TODO: gstreamer based test (could be used to play music files so
 * it has actual merit. Also possibly to simulate network effects?)
 * */

SpicePlaybackInstance playback_instance;


#define TEST_NAME SPICE_SERVER_CHANNEL_PLAYBACK
#define ARGS "<port>"

static const SpiceBaseInterface base = {
    .type          = SPICE_INTERFACE_PLAYBACK,
    .description   = "test playback",
    .major_version = SPICE_INTERFACE_PLAYBACK_MAJOR,
    .minor_version = SPICE_INTERFACE_PLAYBACK_MINOR,
};

uint32_t *frame;
uint32_t num_samples;
SpiceTimer *playback_timer;
int playback_timer_ms;
SpiceCoreInterface *core;

static void get_frame(void)
{
    if (frame) {
        return;
    }
    spice_server_playback_get_buffer(&playback_instance, &frame, &num_samples);
    playback_timer_ms = num_samples ?
                        1000 * num_samples / SPICE_INTERFACE_PLAYBACK_FREQ
                        : 100;
}

void playback_timer_cb(/*SPICE_GNUC_UNUSED*/ void *opaque)
{
    static int t = 0;
    static uint64_t last_sent_usec = 0;
    static uint64_t samples_to_send;
    uint32_t i;
    struct timeval cur;
    uint64_t cur_usec;

    get_frame();
    if (!frame) {
        /* continue waiting until there is a channel */
        core->timer_start(playback_timer, 100);
        return;
    }

    /* we have a channel */
    gettimeofday(&cur, NULL);
    cur_usec = cur.tv_usec + cur.tv_sec * 1e6;
    if (last_sent_usec == 0) {
        samples_to_send = num_samples;
    } else {
        samples_to_send += (cur_usec - last_sent_usec) * SPICE_INTERFACE_PLAYBACK_FREQ / 1e6;
    }
    last_sent_usec = cur_usec;
    while (samples_to_send > num_samples && frame) {
#if 0
    printf("samples_to_send = %d\n", samples_to_send);
#endif
        samples_to_send -= num_samples;
        for (i = 0 ; i < num_samples; ++i) {
            frame[i] = (((uint16_t)((1<<14)*sin((t+i)/10))) << 16) + (((uint16_t)((1<<14)*sin((t+i)/10))));
        }
        t += num_samples;
        if (frame) {
            spice_server_playback_put_samples(&playback_instance, frame);
            frame = NULL;
        }
        get_frame();
    }
    core->timer_start(playback_timer, playback_timer_ms);
}

/*libbd-spice-forward/basic_event_loop.c*/
SpiceCoreInterface *basic_event_loop_init(void);
void basic_event_loop_mainloop(void);

int test_libspice_server_channel_playback_main(int argc, char ** argv)
{
	if(argc < 2 || argc > 1 && 0 == strcmp(argv[1], "help")){
		printf("%s %s %s\n", argv[0], TEST_NAME,  ARGS);
		return 0;
	}

	int port = atoi(argv[1]);
    SpiceServer *server = spice_server_new();
    core = basic_event_loop_init();

    spice_server_set_port(server, port);
    fprintf(stderr, "listening on port %d\n", port);
    spice_server_set_noauth(server);
    spice_server_init(server, core);

    playback_instance.base.sif = &base;
    spice_server_add_interface(server, &playback_instance.base);
    spice_server_playback_start(&playback_instance);

    playback_timer_ms = 100;
    playback_timer = core->timer_add(playback_timer_cb, NULL);
    core->timer_start(playback_timer, playback_timer_ms);

    basic_event_loop_mainloop();

    return 0;
}
