/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2017/9/11
 *
 * net/socket
 * */
#include <unistd.h>
#include "hp_net.h"
#include "net_util.h"   /* get_if_addrs */
#include <sys/ioctl.h>  /* ioctl */
#include <arpa/inet.h>	/* inet_pton */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <sys/uio.h>      /* iovec, writev, ... */
#include <stdio.h>
#include <stdlib.h>     /* free */
#include <string.h>     /* strerror */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include <errno.h>

#define LISTENQ 512  /* for listen() */

int listen_socket_create(int port, int n)
{
	int fd;
	struct sockaddr_in	servaddr = { 0 };

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		hp_log(stderr, "%s: socket error('%s')\n", __FUNCTION__, strerror(errno));
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        hp_log(stderr, "%s: setsockopt SO_REUSEADDR: %s", __FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }
	if(bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		hp_log(stderr, "%s: bind error('%s'), port=%d\n"
				, __FUNCTION__, strerror(errno), port);
		close(fd);
		return -1;
	}

	if(listen(fd, n) < 0){
		hp_log(stderr, "%s: listen error(%s), port=%d\n"
				, __FUNCTION__, strerror(errno), port);
		close(fd);
		return -1;
	}
	return fd;
}

char * get_ipport_cstr(int sockfd, char * buf)
{
	struct sockaddr_in cliaddr = { 0 };
	socklen_t len;
	getsockname(sockfd, (struct sockaddr * )&cliaddr, &len);
	if(!buf){
		static char sbuf[64] = "ip:port";
		buf = sbuf;
	}
	char ip[32];
	inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));
	snprintf(buf, 64, "%s:%d", ip, ntohs(cliaddr.sin_port));
	return buf;
}

char * get_ipport_cstr2(struct sockaddr_in * addr, char const * sep, char * buf, int len)
{
	if(!(addr && sep && buf && len > 0))
		return 0;

	buf[0] = '\0';

	char ip[64];
	inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
	snprintf(buf, 64, "%s%s%d", ip, sep, ntohs(addr->sin_port));

	return buf;
}

char * get_ipport(int sockfd, char * ip, int len, int * port)
{
	if(!(ip && port)) return ip;

	struct sockaddr_in cliaddr = { 0 };
	socklen_t slen;
	getsockname(sockfd, (struct sockaddr * )&cliaddr, &slen);
	inet_ntop(AF_INET, &cliaddr.sin_addr, ip, len);

	*port = ntohs(cliaddr.sin_port);

	return ip;
}

int connect_socket_create(char const * ip, int port, int rcvbuf, int sndbuf)
{
	if(!ip)
		return -1;

    struct sockaddr_in servaddr = { 0 };
    int fd = -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0){
        hp_log(stderr, "%s: socket failed(%s), ip='%s', port=%d.\n", __FUNCTION__, strerror(errno), ip, port);
        return -1;
    }
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0){
    	hp_log(stderr, "%s: inet_pton failed(%s), ip='%s', port=%d.\n", __FUNCTION__, strerror(errno), ip, port);
    	close(fd);
        return -1;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

	int oldsz;
	if(rcvbuf > 0 && fd_set_recvbuf(fd, &oldsz, rcvbuf) != 0)
		hp_log(stderr, "%s: WARNING, fd_set_recvbuf failed, rcvbuf=%d\n" , __FUNCTION__, rcvbuf);
	if(sndbuf > 0 && fd_set_sendbuf(fd, &oldsz, sndbuf) != 0)
		hp_log(stderr, "%s: WARNING, fd_set_sendbuf failed, sndbuf=%d\n" , __FUNCTION__, sndbuf);

    if(connect(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0){
    	hp_log(stderr, "%s: connect error(%d/%s), ip='%s', port=%d.\n", __FUNCTION__, errno, strerror(errno), ip, port);
    	close(fd);
        return -1;
    }

	unsigned long sockopt = 1;
	if(ioctl(fd, FIONBIO, &sockopt) < 0)
		hp_log(stderr, "%s: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, fd);

	return fd;
}

int client_socket_create(int port)
{
	int fd = listen_socket_create(port, LISTENQ);
	if(fd > 0) {
		unsigned long sockopt = 1;
		if(ioctl(fd, FIONBIO, &sockopt) < 0)
			hp_log(stderr, "%s: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, fd);
	}
	return fd;
}

/*
 * @note: "the listen problem":
 * if 3 proxyBs connect to 2 proxyAs, all of proxyB will log "connected...", but
 * actually only 2 proxyB truly connected and the left 1 "queued", see listen()
 * for why
 * */
int proxy_socket_create(int port) { return listen_socket_create(port, LISTENQ); }

/* @return: return the connected fd on success(including errno == EINPROGRESS), -1 on error,
 * if errno == EINPROGRESS, set @param f to 1 */
int client_connect_socket_create(char const * ip, int port, int * f)
{
    struct sockaddr_in servaddr = { 0 };
    int fd_socket = -1;

    fd_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_socket < 0){
        hp_log(stderr, "%s: create socket failed(%s), ip='%s', port=%d.\n", __FUNCTION__, strerror(errno), ip, port);
        return -1;
    }
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0){
    	hp_log(stderr, "%s: inet_pton failed(%s), ip='%s', port=%d\n", __FUNCTION__, strerror(errno), ip, port);
    	close(fd_socket);
        return -1;
    }
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    unsigned long sockopt = 1;
	ioctl(fd_socket, FIONBIO, &sockopt);

	if(connect(fd_socket, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0){
		if(errno != EINPROGRESS){
			hp_log(stderr, "%s: connect error(%s), ip='%s', port=%d.\n", __FUNCTION__, strerror(errno), ip, port);
			close(fd_socket);
			return -1;
		}
		if(f) *f = 1;
    }

	return fd_socket;
}

int fd_set_recvbuf(int fd, int * oldsz, int newsz)
{
	if(!(fd > 0)) return -1;
	if(!(newsz > 0)) return 0;

	int err;
	int len, * plen = &len;
	socklen_t nOptLen = sizeof(len);
	if(oldsz) plen = oldsz;

	err = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)plen, &nOptLen);
	if(!(err == 0)) {
		hp_log(stderr, "%s: getsockopt(SO_RCVBUF) failed, fd=%d, new=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, fd, newsz, errno, strerror(errno));
		return -1;
	}

	err = setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(char*)&newsz, nOptLen);
	if(!(err == 0)) {
		hp_log(stderr, "%s: setsockopt(SO_RCVBUF) failed, fd=%d, old=%d, new=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, fd, len, newsz, errno, strerror(errno));
		return -1;
	}

	return 0;
}

int fd_set_sendbuf(int fd, int * oldsz, int newsz)
{
	if(!(fd > 0)) return -1;
	if(!(newsz > 0)) return 0;

	int err;
	int len, * plen = &len;
	socklen_t nOptLen = sizeof(len);
	if(oldsz) plen = oldsz;

	err = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)plen, &nOptLen);
	if(!(err == 0)) return -1;

	err = setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(char*)&newsz, nOptLen);
	if(!(err == 0)) return -1;

	return 0;
}

size_t iovec_total_bytes(struct iovec * vec, int count)
{
	size_t bytes = 0;
	int i = 0;
	for(; i < count; ++i)
		bytes += vec[i].iov_len;

	return bytes;
}

/* NOTE: will change @param vec if needed */
void before_writev(struct iovec * vec, int count,
		size_t nwrite, int * vec_n)
{
	size_t r = 0;
	int i = *vec_n;
	for(; i != count; ++i){
		r += vec[i].iov_len;
		if(nwrite < r){
			size_t left = r - nwrite;
			vec[i].iov_base = (char *)(vec[i].iov_base) + vec[i].iov_len - left;
			vec[i].iov_len = left;

			*vec_n = i;
			break;
		}
	}
}
/*!
 * call writev once would be the best; but writev dosen't guarantee this,
 * as many other write* functions behave
 *
 * NOTE: will change @param vec if needed
 * @return: bytes written, < 0 on failure
 * */
ssize_t do_writev(int fd, struct iovec * vec, int count, size_t bytes)
{
	if(!vec) return -1;

	ssize_t nwrite  = 0;
	int vec_n = 0;
	for(;;){
		ssize_t w = writev(fd, vec + vec_n, count - vec_n);
		if ( w <= 0) {
			if (w < 0 && (errno == EINTR || errno == EAGAIN))
				continue;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nwrite += w;
		if(nwrite == bytes)
			return nwrite;

		before_writev(vec, count, w, &vec_n);
	}

	return nwrite;
}

ssize_t writev_a(int fd, int * err, struct iovec * vec, int count, int * n, size_t bytes)
{
	if(!(err && vec && n)) return -1;

	ssize_t nwrite  = 0;
	*n = 0;

	for(;;){
		ssize_t w = writev(fd, vec + *n, count - *n);
		if ( w <= 0) {
			if (w < 0 && (errno == EINTR || errno == EAGAIN))
				*err = EAGAIN;
			else{
				if(!((errno == EINTR || errno == EAGAIN)))
					*err = errno;
				else *err = EIO;
			}
			break;
		}

		nwrite += w;
		if(nwrite == bytes){
			*n = count;
			*err = 0;
			break;
		}

		before_writev(vec, count, w, n);
	}

	return nwrite;
}

size_t write_a(int fd, int * err, char const * buf, size_t len)
{
	if(!(err && buf)) return -1;
	if(len == 0)
		return 0;

	size_t nwrite  = 0;

	for(;;){
		ssize_t w = write(fd, buf + nwrite, len - nwrite);
		if ( w <= 0) {
			if (w < 0 && (errno == EINTR || errno == EAGAIN))
				*err = EAGAIN;
			else{
				if(!((errno == EINTR || errno == EAGAIN)))
					*err = errno;
				else *err = -errno;
			}
			break;
		}

		nwrite += w;
		if(nwrite == len){
			*err = 0;
			break;
		}
	}
	return nwrite;
}

static void before_readv(struct iovec * vec, int count,
		size_t nread, int * vec_n)
{
	size_t r = 0;
	int i = *vec_n;
	for(; i != count; ++i){
		r += vec[i].iov_len;
		if(nread < r){
			size_t left = r - nread;
			vec[i].iov_base = (char *)(vec[i].iov_base) + vec[i].iov_len - left;
			vec[i].iov_len = left;

			*vec_n = i;
			break;
		}
	}
}

ssize_t do_readv(int fd, struct iovec * vec, int count, size_t bytes)
{
	if(!vec) return -1;

	size_t nread  = 0;
	int vec_n = 0;
	for(;;){
		ssize_t r = readv(fd, vec + vec_n, count - vec_n);
		if ( r < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;		/* and call read() again */
			else
				return(-1);
		}
		else if (r == 0)
			break;				/* EOF */

		nread += r;
		if(nread == bytes)
			return nread;

		before_readv(vec, count, r, &vec_n);
	}
	return nread;
}

size_t readv_a(int fd, int * err, struct iovec * vec, int count, int * n, size_t bytes)
{
	if(!(vec && n && err)) return 0;

	*n = 0;
	size_t nread  = 0;
	for(;;){
		ssize_t r = readv(fd, vec + *n, count - *n);
		if ( r < 0) {
			/* and call read() again later */
			if (errno == EINTR || errno == EAGAIN)
				*err = EAGAIN;
			else
				*err = errno;
			break;
		}
		else if (r == 0){ /* EOF */
			*err = -1;
			break;
		}
		nread += r;
		if(nread == bytes){
			*n = count;
			*err = 0;
			break;
		}

		before_readv(vec, count, r, n);
	}
	return nread;
}

size_t read_a(int fd, int * err, char * buf, size_t len, size_t bytes)
{
	if(!(fd >= 0 && err && buf && len > 0))
		return 0;

	size_t nread  = 0;
	for(;;){
		ssize_t r = read(fd, buf + nread, len - nread);
		if ( r < 0) {
			/* and call read() again later */
			if (errno == EINTR || errno == EAGAIN)
				*err = EAGAIN;
			else
				*err = errno;
			break;
		}
		else if (r == 0){ /* EOF */
			*err = -1;
			break;
		}
		nread += r;
		if(nread == bytes){
			*err = 0;
			break;
		}
	}
	return nread;
}
/*
 * Write "n" bytes to a descriptor.
 * NOTE: from book <unpv13e>, sample url: https://github.com/k84d/unpv13e
 * */
ssize_t	writen(int fd, const char *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && (errno == EINTR || errno == EAGAIN))
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

/*
 * Read "n" bytes from a descriptor
 * NOTE: from book <unpv13e>, sample url: https://github.com/k84d/unpv13e
 * */
ssize_t	readn(int fd, void *vptr, size_t n)
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = (char *)vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR || errno == EAGAIN)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}

//#include <stdio.h>
//#include <string.h>
//#include <arpa/inet.h>
#include <netdb.h>
//#include <stdlib.h>

/* NOTE:
 * from http://blog.csdn.net/small_qch/article/details/16805857
*/
int get_ip_from_host(char *ipbuf, const char *host, int maxlen)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    /* TODO: inet_pton */
    if (inet_aton(host, &sa.sin_addr) == 0)
    {
        struct hostent *he;
        he = gethostbyname(host);
        if (he == NULL)
            return -1;
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strncpy(ipbuf, inet_ntoa(sa.sin_addr), maxlen);
    return 0;
}

/* @return: 1 on yes, 0 on no, -1 on error */
int is_exist_in_local_addrs(char const * ip)
{
	if(!(ip && ip[0] != '\0')) return -1;

	char ips[64][64];
	int count = 64;
	int result  = get_if_addrs(ips[0], &count, 64);
	if(result != 0)
		return -1;

	int i = 0;
	for(; i < count; ++i){
		if(strncmp(ips[i], ip, strlen(ips[i])) == 0)
			return 1;
	}
	return 0;
}

/*
 * @return: 0 on false; else true */
int netutil_same_subnet(int mask, char const * ip1, char const * ip2)
{
	if(!(ip1 && ip2))
		return 0;
	if(mask < 0) {
		char const * netmask = "255.255.255.0";
		struct sockaddr_in addr1 = { 0 }, addr2 = { 0 }, addr = { 0 };
		if(inet_pton(AF_INET, ip1, &addr1.sin_addr) <= 0 ||
				inet_pton(AF_INET, ip2, &addr2.sin_addr) <= 0 ||
				inet_pton(AF_INET, netmask, &addr.sin_addr) <= 0){
			hp_log(stderr, "%s: inet_pton failed, error='%s', ip1='%s', ip2='%s', mask='%s'.\n"
					, __FUNCTION__, strerror(errno)
					, ip1, ip2, netmask);
			return 0;
		}
		return (addr1.sin_addr.s_addr & addr.sin_addr.s_addr) ==
				(addr2.sin_addr.s_addr & addr.sin_addr.s_addr)? 1 : 0;
	}
	fprintf(stderr, "%s: TODO: no implementation yet!, set mask to -1 to use default subnet mask: '255.255.255.0'\n", __FUNCTION__);
	return 0;
}

/* same as @see netutil_same_subnet */
int netutil_same_subnet3(int mask, uint32_t ip1, char const * ip2)
{
	if(!(ip1 && ip2))
		return 0;
	if(mask < 0) {
		char const * netmask = "255.255.255.0";
		struct sockaddr_in addr2 = { 0 }, addr = { 0 };
		if(inet_pton(AF_INET, ip2, &addr2.sin_addr) <= 0 ||
				inet_pton(AF_INET, netmask, &addr.sin_addr) <= 0){
			hp_log(stderr, "%s: inet_pton failed, error='%s', ip1='%s', ip2='%s', mask='%s'.\n"
					, __FUNCTION__, strerror(errno)
					, ip1, ip2, netmask);
			return 0;
		}
		return (ip1 & addr.sin_addr.s_addr) ==
				(addr2.sin_addr.s_addr & addr.sin_addr.s_addr)? 1 : 0;
	}
	fprintf(stderr, "%s: TODO: no implementation yet!, set mask to -1 to use default subnet mask: '255.255.255.0'\n", __FUNCTION__);
	return 0;
}

/* same as @see netutil_same_subnet */
static int netutil_same_subnet2(int mask, char const * ip1, uint32_t ip2)
{
	if(mask < 0) {
		char const * netmask = "255.255.255.0";
		struct sockaddr_in addr1 = { 0 }, addr = { 0 };
		if(inet_pton(AF_INET, ip1, &addr1.sin_addr) <= 0 ||
				inet_pton(AF_INET, netmask, &addr.sin_addr) <= 0){
			hp_log(stderr, "%s: inet_pton failed, error='%s', ip1='%s', ip2=%d, mask='%s'.\n"
					, __FUNCTION__, strerror(errno)
					, ip1, ip2, netmask);
			return 0;
		}
		return (addr1.sin_addr.s_addr & addr.sin_addr.s_addr) ==
				(ip2 & addr.sin_addr.s_addr)? 1 : 0;
	}
	fprintf(stderr, "%s: TODO: no implementation yet!, set mask to -1 to use default subnet mask: '255.255.255.0'\n", __FUNCTION__);
	return 0;

}
/* @param ips: ':' seperated IPs, e.g. '172.28.0.170:172.19.255.95'
 * @return: 0 on false; else true */
int netutil_in_same_subnet(int mask, char const * ips, uint32_t ip)
{
	if(!(ips && ips[0] != '\0')) return 0;

	char buf[512];
	strncpy(buf, ips, sizeof(buf) - 2);
	char * ptr = strchr(buf, '\0');
	*ptr = ':';
	*(ptr + 1) = '\0';

	char * p = buf, * q = strchr(p, ':');
	for(; (q = strchr(p, ':')); p = q + 1){
		if(p == q)
			continue;
		*q = '\0';
		if(netutil_same_subnet2(mask, p, ip))
			return 1;
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef NDEBUG
#include <string.h>   /* memset */
#include <sys/stat.h> /* stat */
#include <stdlib.h>   /* malloc */
#include <assert.h>

int test_net_main(int argc, char ** argv)
{
	{
		char buf[64];
		struct sockaddr_in addr = { 0 };
		inet_pton(AF_INET, "172.28.0.59", &addr.sin_addr);
		assert(strcmp(get_ipport_cstr2(&addr, ":", buf, sizeof(buf)), "172.28.0.59:0") == 0);
	}
	{
		char buf[64];
		struct sockaddr_in addr = { 0 };
		inet_pton(AF_INET, "172.28.0.59", &addr.sin_addr);
		assert(strcmp(get_ipport_cstr2(&addr, " ", buf, sizeof(buf)), "172.28.0.59 0") == 0);
	}
	{
		char buf[64];
		struct sockaddr_in addr = { 0 };
		inet_pton(AF_INET, "172.28.0.59", &addr.sin_addr);
		assert(strcmp(get_ipport_cstr2(&addr, "::", buf, sizeof(buf)), "172.28.0.59::0") == 0);
	}
	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "172.28.0.59", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95", addr.sin_addr.s_addr);
	assert(r); }

	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "172.19.255.88", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95", addr.sin_addr.s_addr);
	assert(r); }

	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "173.19.255.88", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95", addr.sin_addr.s_addr);
	assert(!r); }

	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "172.19.255.100", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95:192.168.1.140", addr.sin_addr.s_addr);
	assert(r); }

	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "192.168.1.1", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95:192.168.1.140", addr.sin_addr.s_addr);
	assert(r); }

	{struct sockaddr_in addr = { 0 };
	inet_pton(AF_INET, "191.168.1.1", &addr.sin_addr);
	int r = netutil_in_same_subnet(-1, "172.28.0.170:172.19.255.95:192.168.1.140", addr.sin_addr.s_addr);
	assert(!r); }

	int r = netutil_same_subnet(-1, "172.28.0.170", "172.28.0.4");
	assert(r);
	r = netutil_same_subnet(-1, "172.28.0.170", "195.28.0.4");
	assert(!r);
	{
		struct sockaddr_in	servaddr = { 0 };
		fprintf(stdout, "%s: sockaddr_in, ip_len=%zu, port_len=%zu\n", __FUNCTION__
				, sizeof(servaddr.sin_addr.s_addr), sizeof(servaddr.sin_port));
	}
	{
		struct iovec vec[2] = { {0, 0}, {0, 0} };
		FILE * f = fopen("test_writev_0_count", "w");
		assert(f);
		int fd = fileno(f);
		assert(fd > 0);

		int r = writev(fd, vec, 0);
		assert(r == 0);
	}
	int a = 0;
	if(a == 0){
		hp_log(stdout, "%s:a==0\n", __FUNCTION__);
		a = 1;
	}
	else if(a == 1){
		hp_log(stdout, "%s:a==1\n", __FUNCTION__);
	}
	return 0;

	char ipbuf[128];
	char const * host = "www.baidu.com";
	get_ip_from_host(ipbuf, host, 128);
	printf("%s: host='%s', ip: %s\n", __FUNCTION__, host, ipbuf);


	char buf[1024] = "GET / HTTP/1.1\r\n\r\n";
	int fd = connect_socket_create(ipbuf, 80, 0, 0);

	ssize_t nwrite = write(fd, buf, strlen(buf));
	if(nwrite <= 0){
		return -1;
	}
	hp_log(stdout, "%s: sent http request '%s' to host '%s'\n"
					, __FUNCTION__, buf, host);

	ssize_t nread = read(fd, buf, sizeof(buf));
	if(nread > 0){
		buf[nread] = '\0';
		hp_log(stdout, "%s: recv response from '%s', content='%p', len=%zu\n"
								, __FUNCTION__, host, buf, strlen(buf));
	}

	return 0;
}
#endif /* NDEBUG */
