/*!
 * the system api 'epoll' test
 * @author hongjun.liao <docici@126.com>, @date 2018/1/4
 *
 * doc:
 * https://zhuanlan.zhihu.com/p/27050330
 * epoll通过epoll_create创建一个用于epoll轮询的描述符，通过epoll_ctl添加/修改/删除事件，
 * 通过epoll_wait检查事件，epoll_wait的第二个参数用于存放结果。
 * epoll与select、poll不同，首先，其不用每次调用都向内核拷贝事件描述信息，
 * 在第一次调用后，事件信息就会与对应的epoll描述符关联起来。
 * 另外epoll不是通过轮询，而是通过在等待的描述符上注册回调函数，
 * 当事件发生时，回调函数负责把发生的事件存储在就绪事件链表中，最后写到用户空间。
 */
#ifdef __linux__
#include "cpp_test.h"
#include "hp/str_dump.h"
#include <sys/epoll.h>  /* epoll* */
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdlib.h> 	/* atoi */
#include <unistd.h>
#include <assert.h>
#include <sys/ioctl.h>  /* ioctl */
#include <sys/uio.h>    /* writev */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>      /* errno */

#define LISTENQ                 20
#define MAX_EVENTS              10

static int euraka_ping = 0;

static int handle_socket_connect(int listenfd)
{
	struct sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);
	int fd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
	if(fd < 0){
		fprintf(stderr, "%s: accept failed\n", __FUNCTION__);
		return -1;
	}
	unsigned long sockopt = 1;
	if(ioctl(fd, FIONBIO, &sockopt) < 0)
		fprintf(stderr, "%s: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, fd);

	char ip[16];
	inet_ntop(AF_INET, &clientaddr.sin_addr, ip, sizeof(ip));

	fprintf(stdout, "%s: connection from '%s', fd=%d\n"
			, __FUNCTION__, ip, fd);
	return fd;
}

static int handle_fd_read(int fd)
{
	char buf[1024];
	ssize_t r = read(fd, buf, sizeof(buf));
	if(r <=0){
		fprintf(stdout, "%s: read failed! fd=%d, errno=%d, error='%s'\n"
			, __FUNCTION__
			, fd
			, errno
			, strerror(errno)
			);
		if(r < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;
		return -1;
	}
	fprintf(stdout, "%s: read done, fd=%d, buf='%s', len=%zd\n"
			, __FUNCTION__, fd, dumpstr(buf, r, 256), r);
	return r;
}

static int handle_fd_write(int fd)
{
	char buf[1024] = "HTTP/1.1 200 OK \r\nContent-Type:Application/json\r\n\r\n{\"status\":\"UP\"}\r\n\r\n";
	ssize_t r = write(fd, buf, strlen(buf));
	if(r <=0){
		if(r < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;
		return -1;
	}
	fprintf(stdout, "%s: write done, fd=%d, buf='%s', len=%zd\n"
		, __FUNCTION__, fd, dumpstr(buf, r, 256), r);
	return r;
}

int hello_epoll_main(int argc, char ** argv)
{
	int port = 8100;
	if(argc > 1)
		port = atoi(argv[1]);

	int listenfd;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "%s: socket error(socket failed)\n", __FUNCTION__);
		return -1;
	}
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    int yes = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "%s: setsockopt SO_REUSEADDR: %s", __FUNCTION__, strerror(errno));
        close(listenfd);
        return -1;
    }

	struct sockaddr_in	servaddr = { 0 };
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s: socket error, errno=%d, error='%s'\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}
	if(listen(listenfd, LISTENQ) < 0){
		fprintf(stderr, "%s: socket error(listen failed)\n", __FUNCTION__);
		return -1;
	}

	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		fprintf(stderr, "%s: epoll_create1 failed, errno=%d, error='%s'\n"
				, __FUNCTION__, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct epoll_event ev, events[MAX_EVENTS];
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
		fprintf(stderr, "%s: epoll_ctl failed, errno=%d, error='%s'\n"
				, __FUNCTION__, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%s: listening on port=%d...\n", __FUNCTION__, port);
	for (;;) {
		int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			fprintf(stderr, "%s: epoll_wait failed, errno=%d, error='%s'\n"
					, __FUNCTION__, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		int n;
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listenfd) {
				int confd = handle_socket_connect(listenfd);

				ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				ev.data.fd = confd;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, confd, &ev) == -1) {
					fprintf(stderr, "%s: epoll_ctl failed, errno=%d, error='%s', fd=%d\n"
							, __FUNCTION__, errno, strerror(errno), confd);
					exit(EXIT_FAILURE);
				}
			} else {
				int fd = events[n].data.fd;
				if(events[n].events & EPOLLIN){
					int r = handle_fd_read(fd);
					if(r < 0){
						fprintf(stderr, "%s: handle_fd_read failed, remove, ret=%d, fd=%d\n"
							, __FUNCTION__, r, fd);

						ev.data.fd = fd;
						if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) != 0)
							fprintf(stderr, "%s: epoll_ctl failed, errno=%d, error='%s', fd=%d\n"
								, __FUNCTION__, errno, strerror(errno), fd);
					}
					euraka_ping = 1;
				}
				if(events[n].events & EPOLLOUT && euraka_ping){
					int r = handle_fd_write(events[n].data.fd);
					if(r < 0){
						fprintf(stderr, "%s: handle_fd_write failed, remove, ret=%d, fd=%d\n"
							, __FUNCTION__, r, fd);

						ev.data.fd = fd;
						if(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) != 0)
							fprintf(stderr, "%s: epoll_ctl failed, errno=%d, error='%s', fd=%d\n"
								, __FUNCTION__, errno, strerror(errno), fd);
					}
					euraka_ping = 0;
				}
			}
		} /* foreach fd */
	} /* endof epoll_wait for */
}
static char const * help_hello_epoll()
{
	return "hello_epoll <port>";
}

struct test_entry hello_epoll = { "hello_epoll", hello_epoll_main, help_hello_epoll };
#else
#warning "epoll system ignored on this platform"
#endif /* __linux__ */
