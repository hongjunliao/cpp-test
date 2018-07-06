/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/16
 *
 * the epoll event system
 * */

#include "hp_epoll.h"   /*  */
#include <sys/epoll.h>  /* epoll_event */
#include <unistd.h>
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */

/////////////////////////////////////////////////////////////////////////////////////

int hp_epoll_init(struct hp_epoll * efds, int n)
{
	if(!efds) return -1;

	int epollfd = epoll_create1(0);;
	if (epollfd == -1) {
		hp_log(stderr, "%s: epoll_create1 failed, errno=%d, error='%s'\n"
				, __FUNCTION__, errno, strerror(errno));
		return -2;
	}
	efds->fd = epollfd;

	efds->ev = (struct epoll_event *)calloc(n, sizeof(struct epoll_event));
	efds->ev_len = n;

	return 0;
}

void hp_epoll_uninit(struct hp_epoll * efds)
{
	if(!efds) return;

	free(efds->ev);
	close(efds->fd);
}

/*
 * NOTE: use the same @param ed when you add and modify the same fd
 * */
int hp_epoll_add(struct hp_epoll * efds, int fd, int events, struct hp_epolld * ed)
{
	if(!(efds && ed)) return -1;

	int ret = 0;

	struct epoll_event evobj;
	evobj.events = events;
	evobj.data.ptr = ed;

	if (epoll_ctl(efds->fd, EPOLL_CTL_ADD, fd, &evobj) == 0){
		return 0;
	}
	else {
		if(errno == EEXIST){
			if (epoll_ctl(efds->fd, EPOLL_CTL_MOD, fd, &evobj) == 0)
				return 0;
		}

		hp_log(stderr, "%s: epoll_ctl failed, epollfd=%d, fd=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, efds->fd, fd, errno, strerror(errno));
		ret = -1;
	}
	return ret;
}

int hp_epoll_del(struct hp_epoll * efds, int fd, int events, struct hp_epolld * ed)
{
	if(!(efds && ed)) return -1;

	struct epoll_event evobj;
	evobj.events = events;
	evobj.data.ptr = ed;

	if (epoll_ctl(efds->fd, EPOLL_CTL_DEL, fd, &evobj) != 0){
		hp_log(stderr, "%s: epoll_ctl failed, fd=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, fd, errno, strerror(errno));
		return -1;
	}

	if(ed->n > 0){
		efds->ev[ed->n - 1].data.ptr = 0;
		ed->n = 0;
	}

	return 0;
}

char * hp_epoll_e2str(int events, char * buf, int len)
{
	if(!buf && len > 0) return 0;

	buf[0] = '\0';
	int n = snprintf(buf, len, "%s%s%s%s"
			, (events & EPOLLERR?   "EPOLLERR | " : "")
			, (events & EPOLLET?    "EPOLLET | " : "")
			, (events & EPOLLIN?    "EPOLLIN | " : "")
			, (events & EPOLLOUT?   "EPOLLOUT | " : "")
			);
	if(buf[0] != '\0' && n >= 3 )
		buf[n - 3] = '\0';

	int left = events & (~(EPOLLERR | EPOLLET | EPOLLIN | EPOLLOUT));
	if(!(left == 0))
		snprintf(buf + n, len - n, " | %d", left);

	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
int test_hp_epoll_main(int argc, char ** argv)
{
	char buf[128] = "";
	assert(strcmp(hp_epoll_e2str(0, buf, sizeof(buf)), "") == 0);
	assert(strcmp(hp_epoll_e2str(EPOLLERR, buf, sizeof(buf)), "EPOLLERR") == 0);
	assert(strcmp(hp_epoll_e2str(EPOLLET, buf, sizeof(buf)), "EPOLLET") == 0);
	assert(strcmp(hp_epoll_e2str(EPOLLIN, buf, sizeof(buf)), "EPOLLIN") == 0);
	assert(strcmp(hp_epoll_e2str(EPOLLOUT, buf, sizeof(buf)), "EPOLLOUT") == 0);

	assert(strcmp(hp_epoll_e2str(EPOLLERR | EPOLLET, buf, sizeof(buf)), "EPOLLERR | EPOLLET") == 0);
	assert(strcmp(hp_epoll_e2str(EPOLLET | EPOLLIN, buf, sizeof(buf)), "EPOLLET | EPOLLIN") == 0);

	assert(strcmp(hp_epoll_e2str(EPOLLERR | EPOLLET | EPOLLIN, buf, sizeof(buf)), "EPOLLERR | EPOLLET | EPOLLIN") == 0);

	assert(strcmp(hp_epoll_e2str(EPOLLERR | EPOLLET | EPOLLIN | EPOLLOUT, buf, sizeof(buf)), "EPOLLERR | EPOLLET | EPOLLIN | EPOLLOUT") == 0);

	static struct hp_epoll ghp_efdsobj = { 0 }, * efds = &ghp_efdsobj;
	hp_epoll_init(efds, 100);
	hp_epoll_uninit(efds);
	hp_epoll_init(efds, 4000);
	hp_epoll_uninit(efds);

	return 0;
}
#endif /* NDEBUG */

