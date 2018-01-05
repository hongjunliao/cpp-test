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
#if 0
#ifdef __linux__
#include <sys/epoll.h>			/* epoll* */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_EVENTS 10

int hello_epoll_main(int argc, char ** argv)
{
	struct epoll_event ev, events[MAX_EVENTS];
	int listen_sock, conn_sock, nfds, epollfd;

	/* Code to set up listening socket, 'listen_sock',
	 (socket(), bind(), listen()) omitted */

	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = listen_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		int n;
		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listen_sock) {
				conn_sock = accept(listen_sock, (struct sockaddr *) &local,
						&addrlen);
				if (conn_sock == -1) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				setnonblocking(conn_sock);
				ev.events = EPOLLIN | EPOLLET;
				ev.data.fd = conn_sock;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else {
				do_use_fd(events[n].data.fd);
			}
		}
	}

}
#endif /* __linux__ */
#endif
