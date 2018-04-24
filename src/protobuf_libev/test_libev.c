/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/24
 *
 * test libev
 * cmd sample:
 * cat /etc/passwd | cpp-test-main libev
 * */
#ifndef WITHOUT_LIBPROTOBUF_LIBEV
#include <unistd.h>     /* read, ... */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */

// a single header file is required
#include "ev.h"

// every watcher type has its own typedef'd struct
// with the name ev_TYPE
ev_io    ev_stdin_watcher;
ev_timer ev_timeout_watcher;

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void ev_stdin_cb (EV_P_ ev_io *w, int revents)
{
	char buf[1024];
	ssize_t n = read(w->fd, buf, sizeof(buf));
	if(n > 0){
		fwrite(buf, sizeof(char), n, stdout);
	}
	else if(n == 0){
	  // for one-shot events, one must manually stop the watcher
	  // with its corresponding stop function.
	  ev_io_stop (EV_A_ w);

	  // this causes all nested ev_run's to stop iterating
//	  ev_break (EV_A_ EVBREAK_ALL);

	}
}

// another callback, this time for a time-out
static void ev_timeout_cb (EV_P_ ev_timer *w, int revents)
{
	fprintf(stdout, "%s: timeout\n", __FUNCTION__);
  // this causes the innermost ev_run to stop iterating
//	ev_break (EV_A_ EVBREAK_ONE);
}

int test_libev_main(int argc, char ** argv)
{
	  // use the default event loop unless you have special needs
	  struct ev_loop *loop = EV_DEFAULT;

	  // initialise an io watcher, then start it
	  // this one will watch for stdin to become readable
	  ev_io_init (&ev_stdin_watcher, ev_stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
	  ev_io_start (loop, &ev_stdin_watcher);

	  // initialise a timer watcher, then start it
	  // simple non-repeating 5.5 second timeout
	  ev_timer_init (&ev_timeout_watcher, ev_timeout_cb, 1.2, 3);
	  ev_timer_start (loop, &ev_timeout_watcher);

	  // now wait for events to arrive
	  ev_run (loop, 0);

	  // break was called, so exit
	  return 0;
}
#else
int test_libev_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: undefine WITHOUT_LIBPROTOBUF_LIBEV and install libprotobuf, libev to enable this test\n"
			, __FUNCTION__);
	return -1;
}
#endif /* WITHOUT_LIBPROTOBUF_LIBEV */

char const * help_test_libev()
{
	return "libev";
}
