/*!
 * This file is PART of libhp project
 * @author hongjun.liao <docici@126.com>, @date 2022/1/8
 *
 * the poll event system
 *
 * */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if !defined(__linux__) && !defined(_MSC_VER)

#include "hp_poll.h"   /*  */
#include "stdio.h"     /* fprintf */
#include <poll.h>  /* poll */
#include <unistd.h>
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include "c-vector/cvector.h"

/////////////////////////////////////////////////////////////////////////////////////

struct hp_bwait{
	int (* fn)(struct hp_poll * po, void * arg);
	void * arg;
};



int hp_poll_run(hp_poll * po, int timeout
		, int (* before_wait)(struct hp_poll * po))
{
	int i, n;

	int runed = 0;
	for(;!po->stop && !runed;){
		if(before_wait){
			if(before_wait != (void *)-1){
				int rc = before_wait(po);
				if(rc != 0) return rc;
			}
			else runed = 1;
		}

		for(i = 0; i < (int)cvector_size(po->bwaits); ++i){
			assert(po->bwaits[i]->fn);
			if(po->bwaits[i]->fn){
				int rc = po->bwaits[i]->fn(po, po->bwaits[i]->arg);
				if(rc != 0) return rc;
			}
		}

		/* timeout: @see redis.conf/hz */
		n = poll(po->fds, po->nfd, timeout);
		if(n < 0){
			if(errno == EINTR || errno == EAGAIN)
				continue;

			fprintf(stderr, "%s: poll failed, errno=%d, error='%s'\n"
					, __FUNCTION__, errno, strerror(errno));
			return -7;
		}
		else if(n == 0) continue;

		for (i = 0; i < po->nfd; ++i) {
			if(po->fds[i].fd < 0) { continue; }
			hp_polld * ev = &po->ctx[i];
			if(ev->fn)
				ev->fn(ev->arg, i);
			}
	} /* */

	return 0;
}

int hp_poll_init(struct hp_poll * po, int n)
{
	if(!po) return -1;
	if(n <= 0) n = 65535;

	memset(po, 0, sizeof(hp_poll));

	cvector_init(po->fds, 2);
	cvector_init(po->ctx, 2);
	cvector_init(po->bwaits, 1);

	size_t i;
	for(i = 0; i < cvector_capacity(po->fds); ++i){
		po->fds[i].fd = -1;
	}

	return 0;
}

void hp_poll_uninit(struct hp_poll * po)
{
	if(!po) return;

	size_t i;
	for(i = 0; i < cvector_size(po->bwaits); ++i){
		free(po->bwaits[i]);
	}
	cvector_free(po->fds);
	cvector_free(po->ctx);
	cvector_free(po->bwaits);
}

/*
 * NOTE: use the same @param ed when you add and modify the same fd
 * */
int hp_poll_add(struct hp_poll * po, int fd, int events, hp_polld const * ed)
{
	if(!(po && ed)) return -1;

	if(!ed->fn)
		fprintf(stderr, "%s: warning: callback NULL, fd=%d\n", __FUNCTION__, fd);

	int ret = 0;

	size_t i;
	for(i = 0; i < cvector_capacity(po->fds); ++i){
		if(po->fds[i].fd < 0){
			po->fds[i].fd = fd;
			po->fds[i].events = events;
			po->ctx[i] = *ed;

			break;
		}
	}

	return ret;
}

int hp_poll_del(struct hp_poll * po, int fd)
{
	if(!(po && fd >= 0)) return -1;

	int ret = 0;

//	struct epoll_event evobj;
//	evobj.events = events;
//	evobj.data.ptr = ed;
//
//	if (epoll_ctl(po->fds, POLL_CTL_DEL, fd, &evobj) != 0){
//		fprintf(stderr, "%s: epoll_ctl failed, fd=%d, errno=%d, error='%s'\n"
//				, __FUNCTION__, fd, errno, strerror(errno));
//		ret = -1;
//	}
//
//	if(ed->n > 0){
//		ed->n = 0;
//	}

	return ret;
}

int hp_poll_add_before_wait(struct hp_poll * po
		, int (* before_wait)(struct hp_poll * po, void * arg), void * arg)
{
	if(!(po && before_wait))
		return -1;

//	hp_bwait * bwait = calloc(1, sizeof(hp_bwait));
//	bwait->fn = before_wait;
//	bwait->arg = arg;
//	cvector_push_back(po->bwaits, bwait);

	return 0;
}

char * hp_poll_e2str(int events, char * buf, int len)
{
	if(!buf && len > 0) return 0;

	buf[0] = '\0';
	int n = snprintf(buf, len, "%s%s%s"
			, (events & POLLERR?   "POLLERR | " : "")
			, (events & POLLIN?    "POLLIN | " : "")
			, (events & POLLOUT?   "POLLOUT | " : "")
			);
	if(buf[0] != '\0' && n >= 3 )
		buf[n - 3] = '\0';

	int left = events & (~(POLLERR  | POLLIN | POLLOUT));
	if(!(left == 0))
		snprintf(buf + n, len - n, " | %d", left);

	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG

static int test_cb(struct pollfd * fd, int index)
{
	assert(fd);

	char buf[128];
	fprintf(stdout, "%s:%s", __FUNCTION__, hp_poll_e2str(fd->events, buf, sizeof(buf)));
	return 0;
}

int test_hp_poll_main(int argc, char ** argv)
{
	int rc = 0;

	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(0, buf, sizeof(buf)), "") == 0); }
	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(POLLERR, buf, sizeof(buf)), "POLLERR") == 0); }
	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(POLLIN, buf, sizeof(buf)), "POLLIN") == 0); }
	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(POLLOUT, buf, sizeof(buf)), "POLLOUT") == 0); }
	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(POLLERR  | POLLIN, buf, sizeof(buf)), "POLLERR | POLLIN") == 0); }
	{ char buf[128] = ""; assert(strcmp(hp_poll_e2str(POLLERR  | POLLIN | POLLOUT, buf, sizeof(buf)), "POLLERR | POLLIN | POLLOUT") == 0); }

	hp_poll ghp_poobj = { 0 }, * po = &ghp_poobj;
	hp_poll_init(po, 100);

	hp_polld ed = {test_cb, test_cb};
	rc = hp_poll_add(po, 2, POLLIN, &ed); assert(rc == 0);
	hp_poll_run(po, 200, 0);
	hp_poll_uninit(po);

	hp_poll_init(po, 4000);
	hp_poll_uninit(po);

	return 0;
}
#endif /* NDEBUG */

#endif /**/
