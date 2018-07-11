/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/7/5
 *
 * test paxos protocol
 * https://github.com/Tencent/phxpaxos
 * */
#include <unistd.h>
#include <sys/stat.h>	 /* fstat */
#include <sys/ioctl.h>   /* ioctl */
#include <arpa/inet.h>   /* inet_pton */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>      /* uint64 */
#include <errno.h>       /* errno */
#include <assert.h>      /* assert */
#include <string.h>
#include "sds/sds.h"
#include "cp_log.h"      /*  */
#include "str_dump.h"    /* dumpstr */
#include "hp_net.h"
#include "hp_epoll.h"    /* hp_epolld */
#include "list.h"        /* list_head */

/* generated from paxos_msg.proto */
#include "paxos_msg.pb-c.h"
/////////////////////////////////////////////////////////////////////////////////////////
#define NODE_MAX                     1024
#define PAXOS_PROPOSE_OK            (1 << 1)
/////////////////////////////////////////////////////////////////////////////////////////

typedef struct paxos_config {
	int          node_id;            /**/
	char const * addr;               /* sample: 127.0.0.1:11111 */
	char const * nodestr;            /* sample: 127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113 */

	char const * ip;                 /* 127.0.0.1 */
	int          port;               /* 1111 */
	char *       nodes[NODE_MAX];    /* sample: 127.0.0.1:11112 */
	int          n_nodes;

	char const * echo_file;
	char *       buf;                 /* inernal use */
} paxos_config;

typedef struct paxos_node {
	int        fd;                    /* fds to peer node */
	hp_epolld  ed;
	int        flags;

	struct sockaddr_in servaddr;
	struct list_head   o_list;        /* paxos_msg */

	struct list_head   list;
} paxos_node;

typedef struct paxos_msg {
	uint64_t           t;
	sds                value;
	Xh__PaxosMsg *     msg;
	struct sockaddr_in addr;
	struct list_head   list;
} paxos_msg;

typedef struct paxos_ctx {
	uint64_t instance_id;
	uint64_t t_try;                  /* current t for proposer */
	sds      value_try;              /* try value for proposer */

	uint64_t t_max;                  /* max t for acceptor  */
	uint64_t t_store;                /* stored t on acceptor */
	sds      value_store;            /* stored value on acceptor */

	struct list_head propose_ok_list;       /* paxos_msg */
	int              n_propose_ok;
	int              n_propose_success;

	time_t           echo_file_chksum;
	char             buf[1024 * 8];

	struct list_head nodes;          /* paxos_node, peer nodes */
	int              n_nodes;
	struct list_head o_list;         /* paxos_msg, replies to peer nodes */
} paxos_ctx;

/////////////////////////////////////////////////////////////////////////////////////////
static paxos_ctx g_paxos_ctx_obj = { 0 };
/*
 * this node
 * */
static paxos_config g_cfg_obj = {
		  1
		, "127.0.0.1:1111"
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

paxos_msg * paxos_msg_new()
{
	paxos_msg * ret = (paxos_msg * )calloc(1, sizeof(paxos_msg));
	return ret;
}
void paxos_msg_del(paxos_msg * msg) { free(msg); }

static void paxos_prepare(paxos_ctx * ctx)
{
	++ctx->t_try;

	ctx->t_store = 0;
	sdsclear(ctx->value_store);

	struct list_head * msg_pos, * msg_next;
	list_for_each_safe(msg_pos, msg_next, &ctx->propose_ok_list){
		paxos_msg * resp = (struct paxos_msg *)list_entry(msg_pos, paxos_msg, list);
    	assert(resp);

    	list_del(&resp->list);
    	sdsfree(resp->value);
    	paxos_msg_del(resp);
	}
	ctx->n_propose_ok = 0;
	ctx->n_propose_success = 0;

	list_for_each_safe(msg_pos, msg_next, &ctx->o_list){
		paxos_msg * req = (struct paxos_msg *)list_entry(msg_pos, paxos_msg, list);
    	assert(req);

    	list_del(&req->list);
    	free(req->msg);
    	paxos_msg_del(req);
	}

	/* FIXME: clear ctx->nodes->o_list */

	paxos_node * node;
	list_for_each_entry(node, &ctx->nodes, list){
		assert(node);

		Xh__PaxosMsg * msg = (Xh__PaxosMsg *)malloc(sizeof(Xh__PaxosMsg));;
		paxos_msg * req = paxos_msg_new();
		req->msg = msg;
		req->addr = node->servaddr;

		xh__paxos_msg__init(msg);
		msg->has_instanceid = 1;
		msg->has_nodeid = 1;
		msg->has_proposalid = 1;
		msg->has_flag = 1;
		msg->has_value = 1;
		msg->instanceid = ctx->instance_id;
		msg->nodeid = g_paxos_cfg->node_id;
		msg->proposalid = ctx->t_try;
		msg->flag = 1;
		msg->value.data = (uint8_t	*)ctx->value_try;
		msg->value.len = sdslen(ctx->value_try);

		list_add_tail(&req->list, &node->o_list);
	}
}

static void paxos_propose(paxos_ctx * ctx)
{
	paxos_node * node;
	list_for_each_entry(node, &ctx->nodes, list){
		assert(node);

		if(!(node->flags & PAXOS_PROPOSE_OK))
			continue;

		Xh__PaxosMsg * msg = (Xh__PaxosMsg *)malloc(sizeof(Xh__PaxosMsg));;
		paxos_msg * req = paxos_msg_new();
		req->msg = msg;
		req->addr = node->servaddr;

		xh__paxos_msg__init(msg);
		msg->has_instanceid = 1;
		msg->has_nodeid = 1;
		msg->has_proposalid = 1;
		msg->has_flag = 1;
		msg->has_value = 1;
		msg->instanceid = g_paxos_ctx->instance_id;
		msg->nodeid = g_paxos_cfg->node_id;
		msg->proposalid = g_paxos_ctx->t_try;
		msg->flag = 3;
		msg->value.data = (uint8_t	*)g_paxos_ctx->value_try;
		msg->value.len = sdslen(g_paxos_ctx->value_try);

		list_add_tail(&req->list, &node->o_list);
	}
}

static void paxos_execute(paxos_ctx * ctx)
{
	paxos_node * node;
	list_for_each_entry(node, &ctx->nodes, list){
		assert(node);

		Xh__PaxosMsg * msg = (Xh__PaxosMsg *)malloc(sizeof(Xh__PaxosMsg));;
		paxos_msg * req = paxos_msg_new();
		req->msg = msg;
		req->addr = node->servaddr;

		xh__paxos_msg__init(msg);
		msg->has_instanceid = 1;
		msg->has_nodeid = 1;
		msg->has_proposalid = 1;
		msg->has_flag = 1;
		msg->has_value = 1;
		msg->instanceid = g_paxos_ctx->instance_id;
		msg->nodeid = g_paxos_cfg->node_id;
		msg->proposalid = g_paxos_ctx->t_try;
		msg->flag = 5;
		msg->value.data = (uint8_t	*)g_paxos_ctx->value_try;
		msg->value.len = sdslen(g_paxos_ctx->value_try);

		list_add_tail(&req->list, &node->o_list);
	}
}

static int propose_ok_list_comp_by_t(void * priv, struct list_head *a, struct list_head *b)
{
	struct paxos_msg * na = (struct paxos_msg *)list_entry(a, struct paxos_msg, list);
	struct paxos_msg * nb = (struct paxos_msg *)list_entry(b, struct paxos_msg, list);
	return nb->t > na->t;
}

static int epoll_handle_paxos_node_fd_io(struct epoll_event * ev)
{
#ifndef NDEBUG
	if(gloglevel > 8){
		char buf[64];
		hp_log(stdout, "%s: fd=%d, events='%s'\n", __FUNCTION__
				, hp_epoll_fd(ev), hp_epoll_e2str(ev->events, buf, sizeof(buf)));
	}
#endif /* NDEBUG */
	if(ev->events & EPOLLIN){
		struct paxos_node * node = hp_epoll_arg(ev);
		assert(node);
		int fd = node->fd;

		for(;;){
			socklen_t addr_len = sizeof(struct sockaddr_in);
			int n = recvfrom(fd, g_paxos_ctx->buf, sizeof(g_paxos_ctx->buf), 0, &node->servaddr, &addr_len);
			if(n > 0){
				Xh__PaxosMsg * msg = xh__paxos_msg__unpack(0, n, (const uint8_t * )g_paxos_ctx->buf);
				if(msg){
					char servaddrstr[64] = "";
					get_ipport_cstr2(&node->servaddr, ":", servaddrstr, sizeof(servaddrstr));

					hp_log(stdout, "%s: fd=%d, from server='%s'"
							", msg={node_id=%zu, instanceid=%zu, proposalid=%zu, flag=%u, value='%s'/%zu}\n"
							, __FUNCTION__
							, node->fd
							, servaddrstr
							, msg->nodeid, msg->instanceid, msg->proposalid, msg->flag
							, dumpstr((char const *)msg->value.data, msg->value.len, msg->value.len)
							, msg->value.len);

//					assert(g_paxos_ctx->instance_id == msg->instanceid);

					if(msg->has_preacceptid && msg->flag == 4){
						++g_paxos_ctx->n_propose_success;

						if(g_paxos_ctx->n_propose_success > g_paxos_ctx->n_nodes / 2){
							size_t clen = sdslen(g_paxos_ctx->value_try);
							hp_log(stdout, "%s: echo_end, chosen_value='%s'/%zu\n"
									, __FUNCTION__
									, dumpstr(g_paxos_ctx->value_try, clen, clen), clen);

							paxos_execute(g_paxos_ctx);
						}
					}
					else if(msg->flag == 2){
						node->flags |= PAXOS_PROPOSE_OK;

						paxos_msg * pmnew = paxos_msg_new();
						pmnew->t = msg->proposalid;
						pmnew->value = sdsnewlen(msg->value.data, msg->value.len);

						list_add_tail(&pmnew->list, &g_paxos_ctx->propose_ok_list);
						++g_paxos_ctx->n_propose_ok;

						if(g_paxos_ctx->n_propose_ok > g_paxos_ctx->n_nodes / 2){
							list_sort(0, &g_paxos_ctx->propose_ok_list, propose_ok_list_comp_by_t);
							struct paxos_msg * pmmax = list_entry(g_paxos_ctx->propose_ok_list.next, paxos_msg, list);
							if(pmmax->t > 0){
								g_paxos_ctx->value_try = sdscpylen(g_paxos_ctx->value_try, pmmax->value, sdslen(pmmax->value));
							}
							paxos_propose(g_paxos_ctx);
						}
					}
					xh__paxos_msg__free_unpacked(msg, 0);
				}
				else hp_log(stderr, "%s: paxos_msg unpack failed, buf='%s'/%d\n", __FUNCTION__, g_paxos_ctx->buf, n);
			}
			else {
				if(n < 0 && (errno == EINTR || errno == EAGAIN)){
					/* read again later */
				}
				else{
					hp_log(stderr, "%s: recvfrom failed, fd=%d, return=%d, errno=%d/'%s'\n", __FUNCTION__
						, fd, n, errno, strerror(errno));
				}
				break;
			}
		}

		if(g_paxos_ctx->n_propose_ok > g_paxos_cfg->n_nodes / 2){

		}
	}

	return 0;
}

static void paxos_ctx_write(paxos_ctx * ctx)
{
	int fd = g_paxos_fd;
	struct list_head * msg_pos, * msg_next;
	list_for_each_safe(msg_pos, msg_next, &ctx->o_list){
		paxos_msg * req = (struct paxos_msg *)list_entry(msg_pos, paxos_msg, list);
    	assert(req);

		size_t msglen = xh__paxos_msg__get_packed_size(req->msg);
		size_t packlen = xh__paxos_msg__pack(req->msg, (uint8_t *)ctx->buf);
		assert(msglen == packlen);

		int n = sendto(fd, ctx->buf, msglen, 0, &req->addr, sizeof(struct sockaddr_in));
		if(n < 0 && (errno == EINTR || errno == EAGAIN))
			continue;

		list_del(&req->list);
		xh__paxos_msg__free_unpacked(req->msg, 0);
		paxos_msg_del(req);
	}

}

static int epoll_handle_paxos_fd_io(struct epoll_event * ev)
{
#ifndef NDEBUG
	if(gloglevel > 8){
		char buf[64];
		hp_log(stdout, "%s: fd=%d, events='%s'\n", __FUNCTION__
				, hp_epoll_fd(ev), hp_epoll_e2str(ev->events, buf, sizeof(buf)));
	}
#endif /* NDEBUG */
	if((ev->events & EPOLLERR)){
		return 0;
	}

	int fd = g_paxos_fd;
	if((ev->events & EPOLLIN)){
		for(;;){
			struct sockaddr_in addr;
			socklen_t addr_len = sizeof(struct sockaddr_in);
			int n = recvfrom(fd, g_paxos_ctx->buf, sizeof(g_paxos_ctx->buf), 0, &addr, &addr_len);
			if(n > 0){
				Xh__PaxosMsg * msg = xh__paxos_msg__unpack(0, n, (const uint8_t * )g_paxos_ctx->buf);
				if(msg){
					char cliaddrstr[64] = "";
					get_ipport_cstr2(&addr, ":", cliaddrstr, sizeof(cliaddrstr));

					hp_log(stdout, "%s: from node='%s'"
							", msg={node_id=%zu, instanceid=%zu, proposalid=%zu, flag=%u, value='%s'/%zu}\n"
							, __FUNCTION__
							, cliaddrstr
							, msg->nodeid, msg->instanceid, msg->proposalid, msg->flag
							, dumpstr((char const *)msg->value.data, msg->value.len, msg->value.len)
							, msg->value.len);

//					assert(g_paxos_ctx->instance_id == msg->instanceid);

					if(msg->flag == 5){
						hp_log(stdout, "%s: echo_end, value='%s'/%zu, value_store='%s'/%zu\n"
								, __FUNCTION__
								, dumpstr((char const *)msg->value.data, msg->value.len, msg->value.len)
								, msg->value.len
								, g_paxos_ctx->value_store, sdslen(g_paxos_ctx->value_store));

						g_paxos_ctx->value_store = sdscpylen(g_paxos_ctx->value_store
														, (char const *)msg->value.data, msg->value.len);
						xh__paxos_msg__free_unpacked(msg, 0);

						continue;
					}

					if(msg->proposalid > g_paxos_ctx->t_max && msg->flag == 1){
						g_paxos_ctx->t_max = msg->proposalid;
						xh__paxos_msg__free_unpacked(msg, 0);

						msg = (Xh__PaxosMsg *)malloc(sizeof(Xh__PaxosMsg));
						xh__paxos_msg__init(msg);
						msg->has_instanceid = 1;
						msg->has_nodeid = 1;
						msg->has_proposalid = 1;
						msg->has_flag = 1;
						msg->has_value = 1;
						msg->instanceid = g_paxos_ctx->instance_id;
						msg->nodeid = g_paxos_cfg->node_id;
						msg->proposalid = g_paxos_ctx->t_store;
						msg->flag = 2;
						msg->value.data = (uint8_t	*)g_paxos_ctx->value_store;
						msg->value.len = sdslen(g_paxos_ctx->value_store);

						size_t msglen = xh__paxos_msg__get_packed_size(msg);
						size_t packlen = xh__paxos_msg__pack(msg, (uint8_t *)g_paxos_ctx->buf);
						assert(msglen == packlen);

						n = sendto(fd, g_paxos_ctx->buf, msglen, 0, &addr, addr_len);
						if(n < 0 && (errno == EINTR || errno == EAGAIN)){
							paxos_msg * reply = paxos_msg_new();
							reply->msg = msg; reply->addr = addr;
							list_add_tail(&reply->list, &g_paxos_ctx->o_list);
						}
						else free(msg);
					}
					else if(msg->proposalid == g_paxos_ctx->t_max && msg->flag == 3){
						g_paxos_ctx->t_store = msg->proposalid;
						g_paxos_ctx->value_store = sdscpylen(g_paxos_ctx->value_store, (const char *)msg->value.data, msg->value.len);
						xh__paxos_msg__free_unpacked(msg, 0);

						msg = (Xh__PaxosMsg *)malloc(sizeof(Xh__PaxosMsg));
						xh__paxos_msg__init(msg);
						msg->has_instanceid = 1;
						msg->has_nodeid = 1;
						msg->has_proposalid = 1;
						msg->has_flag = 1;
						msg->has_preacceptid = 1;
						msg->has_value = 1;
						msg->instanceid = g_paxos_ctx->instance_id;
						msg->nodeid = g_paxos_cfg->node_id;
						msg->proposalid = g_paxos_ctx->t_store;
						msg->flag = 4;
						msg->value.data = (uint8_t	*)g_paxos_ctx->value_store;
						msg->value.len = sdslen(g_paxos_ctx->value_store);

						size_t msglen = xh__paxos_msg__get_packed_size(msg);
						size_t packlen = xh__paxos_msg__pack(msg, (uint8_t *)g_paxos_ctx->buf);
						assert(msglen == packlen);

						n = sendto(fd, g_paxos_ctx->buf, msglen, 0, &addr, addr_len);
						if(n < 0 && (errno == EINTR || errno == EAGAIN)){
							paxos_msg * reply = paxos_msg_new();
							reply->msg = msg; reply->addr = addr;
							list_add_tail(&reply->list, &g_paxos_ctx->o_list);
						}
						else free(msg);
					}
				}
				else{
					hp_log(stderr, "%s: paxos_msg unpack failed, buf='%s'/%d\n", __FUNCTION__
							, dumpstr(g_paxos_ctx->buf, n, n), n);
				}
			}
			else {
				if(n < 0 && (errno == EINTR || errno == EAGAIN)){
					/* read again later */
				}
				else{
					hp_log(stderr, "%s: recvfrom failed, fd=%d, return=%d, errno=%d/'%s'\n", __FUNCTION__
						, fd, n, errno, strerror(errno));
				}
				break;
			}
		}
	}

	if((ev->events & EPOLLOUT)){
		paxos_ctx_write(g_paxos_ctx);
	}
	return 0;
}

paxos_node * paxos_node_new(int fd)
{
	paxos_node * node = (paxos_node *)calloc(1, sizeof(paxos_node));
	assert(node);

	node->fd = fd;
	INIT_LIST_HEAD(&node->o_list);
	hp_epolld_set(&node->ed, node->fd, epoll_handle_paxos_node_fd_io, node, 0, 0);

	return node;
}

void paxos_node_del(paxos_node * node)
{
	if(!node)
		return;

	free(node);
}

int paxos_config_load(paxos_config * cfg, int argc, char ** argv)
{
	if(!(cfg))
		return -1;

	int i;
	char const * loglevelstr = 0, * nodeidstr = 0;
	for(i = 1; i < argc; ++i){
		char const * arg = argv[i];
		char const ** p;
		if(arg[0] == '-'){
			if(!(strlen(arg) >= 2)){
				fprintf(stderr, "%s: unsupported, arg: '%s'\n", __FUNCTION__, arg);
				continue;
			}
			switch(arg[1]){
			case 'i': p = &nodeidstr;    break;
			case 'a': p = &cfg->addr;    break;
			case 'n': p = &cfg->nodestr; break;
			case 'v': p = &loglevelstr;  break;
			case 'f': p = &cfg->echo_file;  break;
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

	if(loglevelstr) gloglevel = atoi(loglevelstr);
	if(nodeidstr)   g_paxos_cfg->node_id = atoi(nodeidstr);

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
				cfg->nodes[cfg->n_nodes++] = p;
			break;
		}

		*q = '\0';
		cfg->nodes[cfg->n_nodes++] = p;

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
	hp_log(stdout, "%s: -i=%d, -a='%s'/%s:%d, -n='%s'/%d, -f='%s', -v=%d"
			"\nbuf='%s'\n"
			, __FUNCTION__
			, cfg->node_id
			, cfg->addr, cfg->ip, cfg->port, cfg->nodestr, cfg->n_nodes
			, cfg->echo_file
			, gloglevel
			, dumpstr(cfg->buf, 512, 64)
		);

}

int paxos_init(paxos_ctx * ctx)
{
	assert(ctx);

	int i;
	bzero(ctx, sizeof(paxos_ctx));
	INIT_LIST_HEAD(&ctx->nodes);
	INIT_LIST_HEAD(&ctx->o_list);
	INIT_LIST_HEAD(&ctx->propose_ok_list);

	++ctx->instance_id;

	ctx->value_try = sdsempty();
	ctx->value_store = sdsempty();

	struct stat fs;
	char const * path = g_paxos_cfg->echo_file;
	if(path){
		if(stat(path, &fs) < 0){
			hp_log(stderr, "%s: stat('%s') failed, errno=%d, error='%s'\n"
					, __FUNCTION__, path, errno, strerror(errno));
			return -1;
		}
		g_paxos_ctx->echo_file_chksum = fs.st_mtim.tv_sec;
	}


	for(i = 0; i < g_paxos_cfg->n_nodes; ++i){
		char const * addr = g_paxos_cfg->nodes[i];

		if(strcmp(addr, g_paxos_cfg->addr) == 0)
			continue;

		char const * port = strchr(addr, ':');
		assert(port);
		char ip[64] = "127.0.0.1";
		strncpy(ip, g_paxos_cfg->nodes[i], port - addr);
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

		inet_pton(AF_INET, ip, &(servaddr->sin_addr));

		list_add_tail(&node->list, &ctx->nodes);
		++ctx->n_nodes;

		hp_epoll_add(g_paxos_epoll, node->fd, EPOLLIN | EPOLLOUT | EPOLLET, &node->ed);
	}

	++ctx->n_nodes; /* self */

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

static int paxos_echo(paxos_ctx * ctx, char const * str, int len)
{
	ctx->value_try = sdscpylen(ctx->value_try, str, len);

	hp_log(stdout, "%s: echo_begin, data='%s'/%d\n"
				, __FUNCTION__, str, strlen(str));

	paxos_prepare(ctx);
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

	static char buf[512] = "hello";
	ssize_t ibytes = read(fileno(stdin), buf, sizeof(buf));
	if(buf[ibytes - 1] == '\n')
		buf[ibytes - 1] = '\0';

	int r = paxos_echo(g_paxos_ctx, buf, (int)ibytes);
	return r;
}

static void paxos_try_echo()
{
	struct stat fs;

	char const * path = g_paxos_cfg->echo_file;
	if(!path) return;

	if(stat(path, &fs) < 0){
		hp_log(stderr, "%s: stat('%s') failed, errno=%d, error='%s'\n"
				, __FUNCTION__, path, errno, strerror(errno));
		return;
	}
	if(fs.st_mtim.tv_sec != g_paxos_ctx->echo_file_chksum){
		g_paxos_ctx->echo_file_chksum = fs.st_mtim.tv_sec;

		FILE * f = fopen(path, "r");
		if(!f){
			hp_log(stderr, "%s: fopen('%s') failed, errno=%d, error='%s'\n"
					, __FUNCTION__, path, errno, strerror(errno));
			return;
		}

		char line[512] = "hello";
		while(fgets(line, sizeof(line), f)){
			char * nline = strrchr(line, '\n');
			if(nline) *nline = '\0';

			char * p = strchr(line, ' ');
			if(!p) continue;
			*p = '\0';

			if(atoi(line) != g_paxos_cfg->node_id)
				continue;

			char const * str = p + 1;
			paxos_echo(g_paxos_ctx, str, strlen(str));
			break;
		}
	}
}

static void epoll_before_wait(struct hp_epoll * efds)
{
	paxos_ctx_write(g_paxos_ctx);

    struct list_head * pos, * next;
    list_for_each_safe(pos, next, &g_paxos_ctx->nodes){
    	paxos_node * node = (struct paxos_node *)list_entry(pos, paxos_node, list);
    	assert(node);

    	if(list_empty(&node->o_list))
    		continue;

		int r = connect(node->fd, (struct sockaddr *)&node->servaddr, sizeof(struct sockaddr_in));
		if(r != 0){
			char addr[64] = "";
			get_ipport_cstr2(&node->servaddr, ":", addr, sizeof(addr));

			hp_log(stderr, "%s: connect error(%d/%s), peer='%s'.\n"
					, __FUNCTION__, errno, strerror(errno), addr);
			continue;
		}

		struct list_head * msg_pos, * msg_next;
		list_for_each_safe(msg_pos, msg_next, &node->o_list){
			paxos_msg * req = (struct paxos_msg *)list_entry(msg_pos, paxos_msg, list);
	    	assert(req);

			size_t msglen = xh__paxos_msg__get_packed_size(req->msg);
			size_t packlen = xh__paxos_msg__pack(req->msg, (uint8_t *)g_paxos_ctx->buf);
			assert(msglen == packlen);

			int n = sendto(node->fd, g_paxos_ctx->buf, msglen, 0, &node->servaddr, sizeof(node->servaddr));
			if(n < 0 && (errno == EINTR || errno == EAGAIN))
				continue;

			list_del(&req->list);
			free(req->msg);
			paxos_msg_del(req);
		}
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
	paxos_config_print(g_paxos_cfg);
#endif /* NDEBUG */

	g_paxos_fd = udp_socket_create(g_paxos_cfg->port);
	if(!(g_paxos_fd > 0))
		return -1;

	hp_epoll_init(g_paxos_epoll, 1024);

	hp_epolld_set(g_paxos_epoll_ed, g_paxos_fd, epoll_handle_paxos_fd_io, 0, 0, 0);
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

		paxos_try_echo();
	} /* end of top loop */

	paxos_uninit(g_paxos_ctx);
	hp_epoll_uninit(g_paxos_epoll);
	close(g_paxos_fd);
	paxos_config_unload(g_paxos_cfg);

	return 0;
}

char const * help_test_paxos()
{
	return "\n"
		   "    -i=INT          node id, msut different from each other\n"
		   "    -a=STRING       address for accept connection from paxos node, sample '127.0.0.1:11111'\n"
		   "    -n=STRING       nodes, sample '127.0.0.1:11111,127.0.0.1:11112,127.0.0.1:11113'\n"
		   "    -v=INT          log level\n"
		   "    -f=STRING       file path for echo test, see .echo_file\n";
}
