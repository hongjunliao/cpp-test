/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/7/5
 *
 * test paxos protocol
 * https://github.com/Tencent/phxpaxos
 * */
#include <unistd.h>
#include <sys/ioctl.h>   /* ioctl */
#include <arpa/inet.h>   /* inet_pton */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>      /* uint64 */
#include <errno.h>       /* errno */
#include <assert.h>      /* assert */
#include <string.h>
#include "cp_log.h"      /*  */
#include "str_dump.h"    /* dumpstr */
#include "hp_net.h"
#include "hp_epoll.h"    /* hp_epolld */
#include "hp_io.h"       /* ht_eto */
#include "list.h"        /* list_head */

/* generated from paxos_msg.proto */
#include "paxos_msg.pb-c.h"
/////////////////////////////////////////////////////////////////////////////////////////
#define NODE_MAX 20
/////////////////////////////////////////////////////////////////////////////////////////

typedef struct paxos_config {
	char const * addr;               /* sample: 127.0.0.1:11111 */
	char const * nodestr;            /* sample: 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 */

	char const * ip;                 /* 127.0.0.1 */
	int          port;               /* 1111 */
	char *       nodes[NODE_MAX];    /* sample: 127.0.0.1:11112 */
	int          nodes_len;

	char *       buf;                 /* inernal use */
} paxos_config;

typedef struct paxos_node {
	int        fd;              /* fds to peer node */
	hp_eto     eto;
	hp_eti     eti;
	hp_epolld  ed;

	struct sockaddr_in servaddr;
	struct list_head   list;
} paxos_node;

typedef struct paxos_ctx {
	hp_eto     eto;
	hp_eti     eti;

	uint64_t t_max;                  /* max t currently */
	uint64_t t;
	int      value;

	struct list_head nodes;     /* paxos_node */

} paxos_ctx;

/////////////////////////////////////////////////////////////////////////////////////////
static paxos_ctx g_paxos_ctx_obj = { 0 };
/*
 * this node
 * */
static paxos_config g_cfg_obj = {
		  "127.0.0.1:1111"
		, "127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113"
};
static hp_epoll g_paxos_epoll_obj = { 0 };
static hp_epolld ghp_httpfd_ed_obj = { 0 };
static hp_epolld g_stdin_ed_obj = { 0 };
/////////////////////////////////////////////////////////////////////////////////////////
/* log level */
int                   gloglevel = 1;

static int            g_paxos_fd = 0;
static paxos_config * g_paxos_cfg = &g_cfg_obj;
static hp_epoll *     g_paxos_epoll = &g_paxos_epoll_obj;
static paxos_ctx *    g_paxos_ctx = &g_paxos_ctx_obj;
static hp_epolld *    g_paxos_epoll_ed = &ghp_httpfd_ed_obj;
static hp_epolld *    g_paxos_stdin_ed = &g_stdin_ed_obj;
/////////////////////////////////////////////////////////////////////////////////////////

static int epoll_handle_node_paxos_msg(struct epoll_event * ev)
{
#ifndef NDEBUG
	if(gloglevel > 2){
		char buf[64];
		hp_log(stdout, "%s: fd=%d, events='%s'\n", __FUNCTION__
				, hp_epoll_fd(ev), hp_epoll_e2str(ev->events, buf, sizeof(buf)));
	}
#endif /* NDEBUG */
	return 0;
}

static int epoll_handle_paxos_msg(struct epoll_event * ev)
{
#ifndef NDEBUG
	if(gloglevel > 2){
		char buf[64];
		hp_log(stdout, "%s: fd=%d, events='%s'\n", __FUNCTION__
				, hp_epoll_fd(ev), hp_epoll_e2str(ev->events, buf, sizeof(buf)));
	}
#endif /* NDEBUG */
	if((ev->events & EPOLLERR)){
		return 0;
	}

	if((ev->events & EPOLLIN)){
		hp_eti_read(&g_paxos_ctx->eti, g_paxos_fd, ev);
	}

	if((ev->events & EPOLLOUT)){
		hp_eto_write(&g_paxos_ctx->eto, g_paxos_fd, ev);
	}
	return 0;
}

static int paxos_msg_pack(char * buf, size_t * len, void * arg)
{
	assert(buf && len && arg);

	struct epoll_event * ev = (struct epoll_event *)arg;
	assert(ev);
	struct paxos_node *  node = (struct paxos_node *)hp_epoll_arg(ev);
	assert(node);


	for(;;){
		size_t buflen = *len;

		if(!(buflen > 0))
			return EAGAIN;

		Xh__PaxosMsg * msg = xh__paxos_msg__unpack(0, buflen, (const uint8_t * )buf);
		if(!msg)
			return EAGAIN;

		hp_log(stdout, "%s: read paxos_msg, msg={flag=%d}\n", __FUNCTION__, msg->flag);

		*len -= xh__paxos_msg__get_packed_size(msg);

		if(msg->proposalid > g_paxos_ctx->t_max){
			g_paxos_ctx->t_max = msg->proposalid;

			xh__paxos_msg__init(msg);
			msg->flag = 1;
			size_t len = xh__paxos_msg__get_packed_size(msg);

			void * msgbuf = malloc(len);
			xh__paxos_msg__pack(msg, (uint8_t *)msgbuf);

			hp_eto_add(&node->eto, msgbuf, len, free, 0);
		}
		else if(msg->proposalid == g_paxos_ctx->t_max){
//			g_paxos_ctx->value = msg->value;
			g_paxos_ctx->value = 10;


			xh__paxos_msg__init(msg);
			msg->flag = 2;
			size_t len = xh__paxos_msg__get_packed_size(msg);

			void * msgbuf = malloc(len);
			xh__paxos_msg__pack(msg, (uint8_t *)msgbuf);

			hp_eto_add(&node->eto, msgbuf, len, free, 0);
		}

		xh__paxos_msg__free_unpacked(msg, 0);
	}

	return 0;
}

paxos_node * paxos_node_new(int fd)
{
	paxos_node * node = (paxos_node *)malloc(sizeof(paxos_node));
	assert(node);

	hp_eti_init(&node->eti, 512);
	hp_eto_init(&node->eto, 512);

	node->eti.pack = paxos_msg_pack;

	node->fd = fd;

	hp_epolld_set(&node->ed, node->fd, epoll_handle_node_paxos_msg, node, &node->eti, &node->eto);
	return node;
}

void paxos_node_del(paxos_node * node)
{
	if(!node)
		return;

	hp_eti_uninit(&node->eti);
	hp_eto_uninit(&node->eto);
	free(node);
}

int paxos_config_load(paxos_config * cfg, int argc, char ** argv)
{
	if(!(cfg))
		return -1;

	int i;
	char const * loglevelstr = 0;
	for(i = 1; i < argc; ++i){
		char const * arg = argv[i];
		char const ** p;
		if(arg[0] == '-'){
			if(!(strlen(arg) >= 2)){
				fprintf(stderr, "%s: unsupported, arg: '%s'\n", __FUNCTION__, arg);
				continue;
			}
			switch(arg[1]){
			case 'a': p = &cfg->addr;    break;
			case 'n': p = &cfg->nodestr; break;
			case 'v': p = &loglevelstr; break;
			default:
				fprintf(stderr, "%s: unsupported, arg: '%s'\n", __FUNCTION__, arg);
				continue;
			}

			if(strlen(arg) == 2){
				if(i + 1 == argc){
					fprintf(stderr, "%s: parameter need value, arg: '%s'\n", __FUNCTION__, arg);
					break;
				}
				else{
					*p = argv[++i];
					continue;
				}
			}
			else
				*p = arg + 2;
		}
		else
			fprintf(stderr, "%s: unsupported, arg: '%s'\n", __FUNCTION__, arg);
	}

	if(loglevelstr)
		gloglevel = atoi(loglevelstr);

	char * p, * q;
	char const * nodestr = cfg->nodestr;

	if(!cfg->buf)
		cfg->buf = calloc(1, strlen(nodestr) * 4);

	strcpy(cfg->buf, cfg->addr);
	p = strchr(cfg->buf, ':');
	if(!p) return -1;
	*p = 0;
	++p;

	cfg->ip = cfg->buf;
	cfg->port = atoi(p);

	strcpy(p, nodestr);
	char const * end = p + strlen(p);
	for(q = p; ;){
		q = strchr(p, ',');
		if(!q) {
			if(end - p > 0)
				cfg->nodes[cfg->nodes_len++] = p;
			break;
		}

		*q = '\0';
		cfg->nodes[cfg->nodes_len++] = p;

		p = ++q;
	}

	return 0;
}

void paxos_config_unload(paxos_config * cfg)
{
	if(cfg)
		free(cfg->buf);
}

void paxos_config_print(paxos_config * cfg)
{
	if(!cfg)
		return;
	hp_log(stdout, "%s: -a='%s'/%s:%d, -n='%s'/%d, -v=%d"
			"\nbuf='%s'\n"
			, __FUNCTION__
			, cfg->addr, cfg->ip, cfg->port, cfg->nodestr, cfg->nodes_len
			, gloglevel
			, dumpstr(cfg->buf, 512, 128)
		);

}

int paxos_init(paxos_ctx * ctx)
{
	assert(ctx);

	int i;
	bzero(ctx, sizeof(paxos_ctx));
	INIT_LIST_HEAD(&ctx->nodes);

	for(i = 0; i < g_paxos_cfg->nodes_len; ++i){
		char const * port = strchr(g_paxos_cfg->nodes[i], ':');
		assert(port);
		++port;

		int fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(fd <= 0) return -1;

		unsigned long sockopt = 1;
		if(ioctl(fd, FIONBIO, &sockopt) < 0){
			hp_log(stderr, "%s: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, fd);
			close(fd);
			return -1;
		}

		paxos_node * node = paxos_node_new(fd);
		assert(node);

		struct sockaddr_in * servaddr = &node->servaddr;

		bzero(servaddr, sizeof(struct sockaddr_in));
		servaddr->sin_family = AF_INET;
		servaddr->sin_port = htons(atoi(port));
		inet_pton(AF_INET, "127.0.0.1", &(servaddr->sin_addr));

		list_add_tail(&node->list, &ctx->nodes);

		hp_epoll_add(g_paxos_epoll, node->fd, EPOLLIN | EPOLLET, &node->ed);
	}

	return 0;
}

void paxos_uninit(paxos_ctx * ctx)
{
	struct list_head * pos, * next;
	list_for_each_safe(pos, next, &g_paxos_ctx->nodes){
		paxos_node * node = (struct paxos_node *)list_entry(pos, paxos_node, list);
		assert(node);

		list_del(&node->list);
		paxos_node_del(node);
	}
}

int paxos_echo(paxos_ctx * ctx, char const * str)
{
	if(!(ctx && str))
		return -1;


	paxos_node * node;
	list_for_each_entry(node, &ctx->nodes, list){
		assert(node);

		Xh__PaxosMsg msgobj, * msg = &msgobj;
		xh__paxos_msg__init(msg);
		msg->flag = 3;

		size_t len = xh__paxos_msg__get_packed_size(msg);

		void * msgbuf = malloc(len);
		xh__paxos_msg__pack(msg, (uint8_t *)msgbuf);

		int r = hp_eto_add(&node->eto, msgbuf, len, free, 0);
		if(r != 0) return -1;
	}

	return 0;
}

static int epoll_handle_stdin(struct epoll_event * ev)
{
#ifndef NDEBUG
	if(gloglevel > 2){
		char buf[64];
		hp_log(stdout, "%s: fd=%d, events='%s'\n", __FUNCTION__
				, hp_epoll_fd(ev), hp_epoll_e2str(ev->events, buf, sizeof(buf)));
	}
#endif /* NDEBUG */

	char buf[512] = "hello";

	int r = paxos_echo(g_paxos_ctx, buf);

	if(gloglevel > 1)
		hp_log(stdout, "%s: echo, data='%s', length=%d\n", __FUNCTION__, buf, strlen(buf));

	return r;
}

static void epoll_before_wait(struct hp_epoll * efds)
{
	struct epoll_event evobj, * ev = &evobj;

    struct list_head * pos, * next;
    list_for_each_safe(pos, next, &g_paxos_ctx->nodes){
    	paxos_node * node = (struct paxos_node *)list_entry(pos, paxos_node, list);
    	assert(node);

    	hp_eto * eto = &node->eto;
    	if(eto->o_items_len > 0){
    		int r = connect(node->fd, (struct sockaddr *)&node->servaddr, sizeof(struct sockaddr_in));
    		if(r != 0){
    			char addr[64] = "";
    			get_ipport_cstr2(&node->servaddr, ":", addr, sizeof(addr));

    			hp_log(stderr, "%s: connect error(%d/%s), peer='%s'.\n"
    					, __FUNCTION__, errno, strerror(errno), addr);
    		}
    	}

		ev->events = EPOLLIN | EPOLLOUT;
		ev->data.ptr = &node->ed;

		if((ev->events & EPOLLIN))
			hp_eti_read(&node->eti, node->fd, ev);
		if((ev->events & EPOLLOUT))
			hp_eto_write(&node->eto, node->fd, ev);
    }
}

int test_paxos_main(int argc, char ** argv)
{
	hp_log(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);

	int i;
	int r = paxos_config_load(g_paxos_cfg, argc, argv);

	if(r != 0)
		return -1;
#ifndef NDEBUG
	assert(g_paxos_cfg->nodes_len == 3);
	paxos_config_print(g_paxos_cfg);
#endif /* NDEBUG */

	g_paxos_fd = udp_socket_create(g_paxos_cfg->port);
	if(!(g_paxos_fd > 0))
		return -1;

	hp_epoll_init(g_paxos_epoll, 1024);

	hp_epolld_set(g_paxos_epoll_ed, g_paxos_fd, epoll_handle_paxos_msg, 0, 0, 0);
	hp_epoll_add(g_paxos_epoll, g_paxos_fd, EPOLLIN | EPOLLET, g_paxos_epoll_ed);

	hp_epolld_set(g_paxos_stdin_ed, fileno(stdin), epoll_handle_stdin, 0, 0, 0);
	hp_epoll_add(g_paxos_epoll, fileno(stdin), EPOLLIN | EPOLLET, g_paxos_stdin_ed);

	paxos_init(g_paxos_ctx);

	if(gloglevel > 2)
		hp_log(stdout, "%s: UDP on port=%d, fd=%d, ready\n", __FUNCTION__, g_paxos_cfg->port, g_paxos_fd);
	for(;;){

		epoll_before_wait(g_paxos_epoll);

		/* timeout: @see redis.conf/hz */
		int n = epoll_wait(g_paxos_epoll->fd, g_paxos_epoll->ev, g_paxos_epoll->ev_len, 100);
		if(n < 0){
			if(errno == EINTR || errno == EAGAIN)
				continue;

			hp_log(stderr, "%s: epoll_wait failed, errno=%d, error='%s'\n"
					, __FUNCTION__, errno, strerror(errno));
			return -7;
		}

#ifndef NDEBUG
		if(gloglevel > 8){
			hp_log(stdout, "%s: epoll_wait done, ret=%d, maxfd=%d\n"
				, __FUNCTION__, n, g_paxos_epoll->ev_len);
		}
#endif /* NDEBUG */

		for (i = 0; i < n; ++i) {
			struct epoll_event * ev = g_paxos_epoll->ev + i;
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

	paxos_config_unload(g_paxos_cfg);
	hp_epoll_uninit(g_paxos_epoll);
	paxos_uninit(g_paxos_ctx);

	return 0;
}

char const * help_test_paxos()
{
	return "\n    --a=STRING       address for accept connection from paxos node, sample '127.0.0.1:11111'\n"
		   "    --n=STRING       nodes, sample '127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113'\n";
}
