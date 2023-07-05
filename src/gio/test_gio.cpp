#ifdef CPP_TEST_WITH_GIO

/*
 * test_gio.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: jun
 */
#ifdef TEST_GIO
#include "bd_test.h"
#include <stdio.h>
#include <gio/gio.h>
#include <glib/giochannel.h>

static gboolean handler_gio_stdin (GIOChannel *source, GIOCondition condition, gpointer data);

static guint g_io_source_stdin = 0;
static GMainLoop *loop;

int test_gio_main(int argc, char **argv)
{
	GIOChannel* stdinc = g_io_channel_unix_new (STDIN_FILENO);
	g_io_source_stdin = g_io_add_watch(stdinc, G_IO_IN, handler_gio_stdin, (void *)0);

    loop = g_main_loop_new(NULL, FALSE);
    fprintf(stderr, "g_io_add_watch(stdin)...\n");
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    return 0;
}

static gboolean handler_gio_stdin (GIOChannel *source, GIOCondition condition, gpointer data)
{
	int fd = g_io_channel_unix_get_fd(source);
	char buf[512 + 1] = "";
	int n = read(fd, buf, 512);
	fprintf(stderr, "%s:%s\n", __FUNCTION__, (n > 0? buf : "(null)"));
	g_source_remove(g_io_source_stdin);
//	g_io_channel_unref(source);
	g_main_loop_quit(loop);
	return 0;
}
#endif /*TEST_GIO*/

#endif

