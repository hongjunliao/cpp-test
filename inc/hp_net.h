/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2017/11/24
 *
 * net/socket
 * */

#ifndef XHHP_NET_H__
#define XHHP_NET_H__

#include "hp_select.h"
#include "hp_epoll.h"

#include <sys/select.h> /* fd_set */
#include <netinet/in.h>	/* sockaddr_in */
#include <stdint.h>     /* uint32_t */

/////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#define new __new
#endif

#include "list.h"       /* for list */


/////////////////////////////////////////////////////////////////////////////////////

/* hp_net.cpp */
int client_socket_create(int port);
int proxy_socket_create(int port);
int udp_socket_create(int port);

int connect_socket_create(char const * ip, int port, int rcvbuf, int sndbuf);
int client_connect_socket_create(char const * ip, int port, int * f);
char * get_ipport_cstr(int sockfd, char * buf);
char * get_ipport_cstr2(struct sockaddr_in * addr, char const * sep, char * buf, int len);
char * get_ipport(int sockfd, char * ip, int len, int * port);
int is_exist_in_local_addrs(char const * ip);

void proxya_clients_fdset(struct list_head * list, int proxyfd, struct fd_sets * fds, struct hp_epoll * efds);

struct CLIENT_NODE;
struct PROXYA_CONTEXT;
struct PROXYB_CONTEXT;
void proxyb_clients_connect_and_fdset(struct PROXYA_CONTEXT * actx, struct PROXYB_CONTEXT * bctx
		, struct fd_sets * fds, struct hp_epoll * efds
		, int (*fn)(struct CLIENT_NODE * client));

int netutil_same_subnet(int mask, char const * ip1, char const * ip2);
int netutil_same_subnet3(int mask, uint32_t ip1, char const * ip2);
int netutil_in_same_subnet(int mask, char const * ips, uint32_t ip);

#ifdef __cplusplus
}
#endif

#endif /* XHHP_NET_H__ */
