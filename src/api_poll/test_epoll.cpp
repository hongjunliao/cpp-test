#ifdef __CYGWIN_GCC__
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "bd_test.h"

using namespace std;

#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

void setnonblocking(int sock) {
	int opts;
	opts = fcntl(sock, F_GETFL);
	if (opts < 0) {
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}
}

int test_epoll_main(int argc, char ** argv) {
	int i, listenfd, connfd, sockfd, epfd, nfds, portnumber;
	ssize_t n;
	char line[MAXLINE];
	socklen_t clilen;

	if (2 == argc) {
		if ((portnumber = atoi(argv[1])) < 0) {
			fprintf(stderr, "Usage:%s portnumber/a/n", argv[0]);
			return 1;
		}
	} else {
		fprintf(stderr, "Usage:%s portnumber/a/n", argv[0]);
		return 1;
	}

	//澹版槑epoll_event缁撴瀯浣撶殑鍙橀噺,ev鐢ㄤ簬娉ㄥ唽浜嬩欢,鏁扮粍鐢ㄤ簬鍥炰紶瑕佸鐞嗙殑浜嬩欢

	struct epoll_event ev, events[20];
	//鐢熸垚鐢ㄤ簬澶勭悊accept鐨別poll涓撶敤鐨勬枃浠舵弿杩扮

	epfd = epoll_create(256);
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr = { 0 };
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//鎶妔ocket璁剧疆涓洪潪闃诲鏂瑰紡

	//setnonblocking(listenfd);

	//璁剧疆涓庤澶勭悊鐨勪簨浠剁浉鍏崇殑鏂囦欢鎻忚堪绗�

	ev.data.fd = listenfd;
	//璁剧疆瑕佸鐞嗙殑浜嬩欢绫诲瀷

	ev.events = EPOLLIN | EPOLLET;
	//ev.events=EPOLLIN;

	//娉ㄥ唽epoll浜嬩欢

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	serveraddr.sin_family = AF_INET;
	char *local_addr = "127.0.0.1";
	inet_aton(local_addr, &(serveraddr.sin_addr)); //htons(portnumber);

	serveraddr.sin_port = htons(portnumber);
	bind(listenfd, (sockaddr *) &serveraddr, sizeof(serveraddr));
	listen(listenfd, LISTENQ);

	for (;;) {
		//绛夊緟epoll浜嬩欢鐨勫彂鐢�

		nfds = epoll_wait(epfd, events, 20, 500);
		//澶勭悊鎵�鍙戠敓鐨勬墍鏈変簨浠�

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == listenfd) //濡傛灉鏂扮洃娴嬪埌涓�涓猄OCKET鐢ㄦ埛杩炴帴鍒颁簡缁戝畾鐨凷OCKET绔彛锛屽缓绔嬫柊鐨勮繛鎺ャ��

					{
				connfd = accept(listenfd, (sockaddr *) &clientaddr, &clilen);
				if (connfd < 0) {
					perror("connfd<0");
					exit(1);
				}
				//setnonblocking(connfd);

				char *str = inet_ntoa(clientaddr.sin_addr);
				cout << "accapt a connection from " << str << endl;
				//璁剧疆鐢ㄤ簬璇绘搷浣滅殑鏂囦欢鎻忚堪绗�

				ev.data.fd = connfd;
				//璁剧疆鐢ㄤ簬娉ㄦ祴鐨勮鎿嶄綔浜嬩欢

				ev.events = EPOLLIN | EPOLLET;
				//ev.events=EPOLLIN;

				//娉ㄥ唽ev

				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
			} else if (events[i].events & EPOLLIN) //濡傛灉鏄凡缁忚繛鎺ョ殑鐢ㄦ埛锛屽苟涓旀敹鍒版暟鎹紝閭ｄ箞杩涜璇诲叆銆�

			{
				cout << "EPOLLIN" << endl;
				if ((sockfd = events[i].data.fd) < 0)
					continue;
				if ((n = read(sockfd, line, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
						close(sockfd);
						events[i].data.fd = -1;
					} else
						std::cout << "readline error" << std::endl;
				} else if (n == 0) {
					close(sockfd);
					events[i].data.fd = -1;
				}
				line[n] = '/0';
				cout << "read " << line << endl;
				//璁剧疆鐢ㄤ簬鍐欐搷浣滅殑鏂囦欢鎻忚堪绗�

				ev.data.fd = sockfd;
				//璁剧疆鐢ㄤ簬娉ㄦ祴鐨勫啓鎿嶄綔浜嬩欢

				ev.events = EPOLLOUT | EPOLLET;
				//淇敼sockfd涓婅澶勭悊鐨勪簨浠朵负EPOLLOUT

				//epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

			} else if (events[i].events & EPOLLOUT) // 濡傛灉鏈夋暟鎹彂閫�

			{
				sockfd = events[i].data.fd;
				write(sockfd, line, n);
				//璁剧疆鐢ㄤ簬璇绘搷浣滅殑鏂囦欢鎻忚堪绗�

				ev.data.fd = sockfd;
				//璁剧疆鐢ㄤ簬娉ㄦ祴鐨勮鎿嶄綔浜嬩欢

				ev.events = EPOLLIN | EPOLLET;
				//淇敼sockfd涓婅澶勭悊鐨勪簨浠朵负EPOLIN

				epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
			}
		}
	}
	return 0;
}

#endif /*__CYGWIN_GCC__*/
