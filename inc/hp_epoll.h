/*!
 * This file is PART of xhhp project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/18
 *
 * the epoll event system
 *
 * NOTE:
 * linux ONLY
 * */

#ifndef XHHP_EPOLL_H__
#define XHHP_EPOLL_H__

#include "hp_io.h"      /* hp_etio */
#include <sys/epoll.h>  /* epoll_event */
#include "list.h"       /* list_head */

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////
struct hp_epolld;
struct hp_epoll;


typedef int (* hp_epoll_cb_t)(struct epoll_event * ev);

typedef struct hp_epolld {
	int                  fd;
	hp_epoll_cb_t        fn;
	void *               arg;
	struct hp_eti *      eti;
	struct hp_eto *      eto;

	int                  n;    /* index when fired */
} hp_epolld;

typedef struct hp_epoll {
	int                  fd;
	struct epoll_event * ev;
	int                  ev_len;
} hp_epoll;

int hp_epoll_init(struct hp_epoll * efds, int n);
void hp_epoll_uninit(struct hp_epoll * efds);
int hp_epoll_add(struct hp_epoll * efds, int fd, int events, struct hp_epolld * ed);
int hp_epoll_del(struct hp_epoll * efds, int fd, int events, struct hp_epolld * ed);
char * hp_epoll_e2str(int events, char * buf, int len);

#define hp_epoll_arg(ev) (((struct hp_epolld *)(ev)->data.ptr)->arg)
#define hp_epoll_fd(ev)  (((struct hp_epolld *)(ev)->data.ptr)->fd)
#define hp_epoll_d(ev)    (((struct hp_epolld *)(ev)->data.ptr))

#define hp_epolld_set(ed, _fd, _fn, _arg, _eti, _eto)  \
	(ed)->fd = _fd; (ed)->fn = _fn; (ed)->arg = _arg;   \
	(ed)->eti = _eti; (ed)->eto = _eto; (ed)->n = 0

#ifdef __cplusplus
}
#endif

#endif /* XHHP_EPOLL_H__ */
