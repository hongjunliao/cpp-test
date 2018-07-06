/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/7/5
 *
 * test paxos protocol
 * https://github.com/Tencent/phxpaxos
 * */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>       /* errno */
#include <string.h>
#include "hp_net.h"
#include "hp_epoll.h"
#include "hp_io.h"

/////////////////////////////////////////////////////////////////////////////////////////
#define NODE_MAX 20
/////////////////////////////////////////////////////////////////////////////////////////

typedef struct paxos_config {
	int    port;
	char * nodes[NODE_MAX];     /* sample: 127.0.0.1:11112 */
	int    nodes_len;
	char * buf;
} paxos_config;

typedef struct paxos_node {
	int t;
	int value;

	int fd;
	hp_eto eto;
} paxos_node;

typedef struct paxos_accepter {
	int t_max;
	int t_store;
	int value;
} paxos_accepter;

typedef struct paxos_ctx {
/* nodes connected me */
	paxos_node nodes[NODE_MAX];
	int        nodes_len;

} paxos_ctx;

/////////////////////////////////////////////////////////////////////////////////////////
/* log level */
#ifndef NDEBUG
int gloglevel = 10;
#else
int gloglevel = 0;
#endif /* NDEBUG */

static paxos_ctx g_paxos_ctx_obj = { 0 }, * g_paxos_ctx = &g_paxos_ctx_obj;

/*
 * this node
 * */
static paxos_node g_node = { 0 };

static paxos_config g_cfg_obj = { 0 }, * g_cfg = &g_cfg_obj;

static struct hp_epoll ghp_efdsobj = { 0 };
static int ghp_httpfd = 0;

static struct hp_epoll * ghp_efds = &ghp_efdsobj;
static struct hp_epolld ghp_httpfd_ed_obj = { 0 },  * ghp_httpfd_ed = &ghp_httpfd_ed_obj;
static struct hp_epolld g_stdin_ed_obj = { 0 },  * g_stdin_ed = &g_stdin_ed_obj;
/////////////////////////////////////////////////////////////////////////////////////////

int paxos_config_load(paxos_config * cfg, char const * str)
{
	if(!(cfg && str))
		return -1;

	cfg->buf = malloc(strlen(str) * 2);
	strcpy(cfg->buf, str);

	char * p, * q, * buf;

	buf = cfg->buf;
	for(p = buf, q = p; ;){
		q = strchr(p, ',');
		if(!q) {
			if(q - p > 0)
				cfg->nodes[cfg->nodes_len++] = p;
			break;
		}

		*q = '\0';
		cfg->nodes[cfg->nodes_len++] = p;

		p = ++q;
	}
}

int paxos_config_unload(paxos_config * cfg)
{

}


static int http_epoll_handle_accepts(struct epoll_event * ev)
{

}

static int epoll_stdin(struct epoll_event * ev)
{
	char buf[512] = "hello";

	paxos_echo(g_paxos_ctx, buf);

}

int paxos_init(paxos_ctx * ctx)
{
	int i;
	for(i = 0; i < g_cfg->nodes_len; ++i){
		char const * port = strchr(g_cfg->nodes[i], ':');
		int fd = connect_socket_create(g_cfg->nodes[i], atoi(port), 0, 0);
	}
}

int paxos_echo(paxos_ctx * ctx, char const * str)
{
	if(!(ctx && str))
		return -1;

	char buf[512] = "hello";
	int i;
	for(i  = 0; i < NODE_MAX; ++i){
		hp_eto * eto = &ctx->nodes[i].eto;
		hp_eto_add(eto, str, strlen(str), 0, 0);

	}
}
/////////////////////////////////////////////////////////////////////////////////////////
static void epoll_before_wait(struct hp_epoll * efds)
{

}

int test_paxos_main(int argc, char ** argv)
{
	hp_log(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);

	int i, n;
	char const * nodes = "127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113";
	paxos_config_load(g_cfg, nodes);

	ghp_httpfd = client_socket_create(1111);
	if(!(ghp_httpfd > 0))
		return -1;

	hp_epoll_init(ghp_efds, 1024);

	hp_epolld_set(ghp_httpfd_ed, ghp_httpfd, http_epoll_handle_accepts, 0, 0, 0);
	hp_epoll_add(ghp_efds, ghp_httpfd, EPOLLIN | EPOLLET, ghp_httpfd_ed);

	hp_epolld_set(ghp_httpfd_ed, fileno(stdin), epoll_stdin, 0, 0, 0);
	hp_epoll_add(ghp_efds, fileno(stdin), EPOLLIN | EPOLLET, g_stdin_ed);

	paxos_init(g_paxos_ctx);

	for(;;){

		epoll_before_wait(ghp_efds);

		/* timeout: @see redis.conf/hz */
		int n = epoll_wait(ghp_efds->fd, ghp_efds->ev, ghp_efds->ev_len, 100);
		if(n < 0){
			if(errno == EINTR || errno == EAGAIN)
				continue;

			hp_log(stderr, "%s: epoll_wait failed, errno=%d, error='%s'\n"
					, __FUNCTION__, errno, strerror(errno));
			return -7;
		}

#ifndef NDEBUG
		if(gloglevel > 16){
			hp_log(stdout, "%s: epoll_wait done, ret=%d, maxfd=%d\n"
				, __FUNCTION__, n, ghp_efds->ev_len);
		}
#endif /* NDEBUG */

		for (i = 0; i < n; ++i) {
			struct epoll_event * ev = ghp_efds->ev + i;
			struct hp_epolld  * evdata = (struct hp_epolld  *)(ev->data.ptr);

			if(!evdata)
				continue;

			evdata->n = i + 1;
			if(evdata->fn){
				evdata->fn(ev);
				if(ev->data.ptr){
					evdata = (struct hp_epolld  *)(ev->data.ptr);
					evdata->n = 0;
				}
				continue;
			}

#ifndef NDEBUG
			if(gloglevel > 0){
				char buf[64];
				hp_log(stderr, "%s: epoll callback NOT set!, fd=%d, events='%s'\n", __FUNCTION__
						, evdata->fd, hp_epoll_e2str(ev->events, buf, sizeof(buf)));
			}
#endif /* NDEBUG */

		} /* for each epoll-ed fd */
	} /* end of top loop */
	return 0;
}

