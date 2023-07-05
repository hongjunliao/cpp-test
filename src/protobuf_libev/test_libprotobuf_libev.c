/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/20
 *
 * test libprotobuf and libev
 *
 * (1)ubuntu: apt install libprotobuf-dev protobuf-c-compiler  protobuf-compiler libprotobuf-c-dev libev-dev libprotoc-dev

 * (2)compile *.proto:
 * protoc http.proto --cpp_out .
 * or
 * protoc-c http.proto --c_out .
 * */
#include <stdio.h>
#ifdef CPP_TEST_WITH_LIBPROTOBUF_LIBEV
#include <unistd.h>     /* read, ... */
#include "http.pb-c.h"
#include "ev.h"
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include "sds/sds.h"    /* sds */
#include "ev.h"         /* libev */

static ev_io    ev_stdin_watcher;
static ev_timer ev_timeout_watcher;

static sds ev_stdin_buf = 0;

#define PB_URL      "http://127.0.0.1/index.html"
#define PB_OUTFILE  "/tmp/test_libprotobuf_libev_main"

static void ev_stdin_cb (EV_P_ ev_io *w, int revents)
{
	char buf[2048];
	ssize_t n = read(w->fd, buf, sizeof(buf));
	if(n > 0){
		if(!ev_stdin_buf)
			ev_stdin_buf = sdsnewlen(buf, n);
		else
			ev_stdin_buf = sdscatlen(ev_stdin_buf, buf, n);
	}
	else if(n == 0){
		HttpMsg * msg2  = 0;
		uint8_t const * pbbuf = (uint8_t const *)ev_stdin_buf;

		msg2 = http_msg__unpack(0, sdslen(ev_stdin_buf), pbbuf);

		if(!(msg2 && strcmp(PB_URL, msg2->url) == 0)){
			fprintf(stderr, "%s: pb unpack failed from file '%s'\n", __FUNCTION__, PB_OUTFILE);
		}
		else{
			fprintf(stdout, "%s: pb unpack from file '%s', msg=%p, url='%s'\n", __FUNCTION__
					,PB_OUTFILE, msg2, msg2->url);
		}

		http_msg__free_unpacked(msg2, 0);
		sdsfree(ev_stdin_buf);
		ev_stdin_buf = 0;

		ev_io_stop (EV_A_ w);
//	  ev_break (EV_A_ EVBREAK_ALL);
	}
}

// another callback, this time for a time-out
static void ev_timeout_cb (EV_P_ ev_timer *w, int revents)
{
	fprintf(stdout, "%s: timeout\n", __FUNCTION__);
//	ev_break (EV_A_ EVBREAK_ONE);
}

int test_libprotobuf_libev_main(int argc, char ** argv)
{
	HttpMsg msg;
	http_msg__init(&msg);
	msg.url = PB_URL;

	size_t N = http_msg__get_packed_size(&msg);
	uint8_t * out = malloc(N * 2);
	size_t r = http_msg__pack(&msg, (uint8_t *)out);
	if(r != N){
		fprintf(stdout, "%s: pack failed, size=%zu, pack_size=%zu\n", __FUNCTION__, N, r);
		free(out);
		return -1;
	}

	FILE * f = fopen(PB_OUTFILE, "w");
	assert(f);
	r = fwrite(out, sizeof(char), N, f);
	if(r != N){
		fprintf(stderr, "%s: fwrite failed, size=%zu, return=%zu, errno=%d, error='%s'\n"
				, __FUNCTION__
				, N, r
				, errno, strerror(errno));
		free(out);
		return -1;
	}
	fclose(f);
	free(out);

	fprintf(stdout, "%s: pb pack to file '%s', msg=%p, url='%s'\n", __FUNCTION__
			, PB_OUTFILE, &msg, msg.url);

	struct ev_loop *loop = EV_DEFAULT;
	ev_io_init (&ev_stdin_watcher, ev_stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
	ev_io_start (loop, &ev_stdin_watcher);

	ev_timer_init (&ev_timeout_watcher, ev_timeout_cb, 1.2, 3);
	ev_timer_start (loop, &ev_timeout_watcher);
	ev_run (loop, 0);

	return 0;
}

#else
int test_libprotobuf_libev_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: undefine WITHOUT_LIBPROTOBUF_LIBEV and install libprotobuf, libev to enable this test\n"
			, __FUNCTION__);
	return -1;
}
#endif /* WITHOUT_LIBPROTOBUF_LIBEV */


char const * help_test_libprotobuf_libev()
{
	return "libprotobuf_libev";
}
