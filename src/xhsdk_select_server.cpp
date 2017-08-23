/*!
 * xhsdk_select_server
 * @author hongjun.liao <docici@126.com>, @date 2017/8/23
 */
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <unistd.h>
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>

#define LISTENQ 1024		   /* for socket/listen */
#define XHSDKCLIENT_MAX 1024   /**/

struct XhSDKClient {
	int fd;
	char ip[16];
};

/* MAX for client, 0 for NOT use */
static XhSDKClient g_xhsdk_clis[XHSDKCLIENT_MAX] = { 0 };
static int xhsdkclient_add(XhSDKClient const & cli);

static int select_on_socket_connect(XhSDKClient & cli);
static int select_on_stdin();
static int select_on_socket_rw(fd_set & fds);

int xhsdk_select_server_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);

	int fd;
	int stdinfd = fileno(stdin);
	struct sockaddr_in	servaddr = { 0 };

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "%s: socket error(socket failed)\n", __FUNCTION__);
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(17000);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(fd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s: socket error(bind failed)\n", __FUNCTION__);
		return -1;
	}
	if(listen(fd, LISTENQ) < 0){
		fprintf(stderr, "%s: socket error(listen failed)\n", __FUNCTION__);
		return -1;
	}

	fd_set rfds;
	int nfds = fd > stdinfd ? fd : stdinfd;
	struct timeval timeout = {3, 0};
	for(;;){
		FD_ZERO(&rfds);
		FD_SET(stdinfd, &rfds);
		FD_SET(fd, &rfds);

		int ret = select(nfds + 1, &rfds, (fd_set *)0, (fd_set *)0, &timeout);
		if(ret == -1){
			fprintf(stderr, "%s: select failed\n", __FUNCTION__);
			break;
		}
		if(ret == 0){
			fprintf(stdout, "%s: select timedout\n", __FUNCTION__);
			continue;
		}

		if (FD_ISSET(fd, &rfds)) {	/* connect */
			XhSDKClient cli = { fd, "" };
			if(select_on_socket_connect(cli) == 0){
				int confd = cli.fd;
				nfds = confd > nfds? confd : nfds;
				FD_SET(confd, &rfds);
			}
		}
		/* socket I/O */
		select_on_socket_rw(rfds);

		if(FD_ISSET(stdinfd, &rfds)){ /* stdin */
			select_on_stdin();
		}
	}

	return 0;
}

static int select_on_socket_connect(XhSDKClient & cli)
{
	sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);
	int confd = accept(cli.fd, (struct sockaddr *)&clientaddr, &len);
	if(confd < 0){
		fprintf(stderr, "%s: accept failed\n", __FUNCTION__);
		return -1;
	}
	cli.fd = confd;

	inet_ntop(AF_INET, &clientaddr.sin_addr, cli.ip, sizeof(cli.ip));
	fprintf(stdout, "%s: connection from '%s'\n", __FUNCTION__, cli.ip);

	if(xhsdkclient_add(cli) != 0){
		fprintf(stderr, "%s: TOO many clients, closing\n", __FUNCTION__);
		close(confd);
		return -2;
	}
	return 0;
}

static int xhsdkclient_add(XhSDKClient const & cli)
{
	for(int i = 0; i < XHSDKCLIENT_MAX; ++i){
		if(g_xhsdk_clis[i].fd == 0){
			g_xhsdk_clis[i] = cli;
			return 0;
		}
	}
	return -1;
}

static int select_on_socket_rw(fd_set & fds)
{
	for(int i = 0; i < XHSDKCLIENT_MAX; ++i){
		int& fd = g_xhsdk_clis[i].fd;
		if( fd != 0 && FD_ISSET(fd, &fds)){
			char buf[1024];
			ssize_t ret = read(fd, buf, sizeof(buf));
			if(ret == -1 || ret == 0){
				if(ret == -1)
					fprintf(stderr, "%s: socket error(read failed)\n", __FUNCTION__);
				else
					fprintf(stdout, "%s: client closed\n", __FUNCTION__);

				FD_CLR(fd, &fds);
				close(fd);
				fd = 0;
				continue;
			}
			buf[ret] = '\0';	/* required */
			fprintf(stdout, "%s: fd=%d, len=%ld, buff='%s'\n", __FUNCTION__, fd, ret, buf);

			size_t t = 0;;
			sprintf(buf, "%zu: hello client", t);

			ret = write(fd, buf, strlen(buf));
			if(ret == -1){
				fprintf(stderr, "%s: write failed, buff='%s'\n", __FUNCTION__, buf);

				FD_CLR(fd, &fds);
				close(fd);
				fd = 0;
			}
		}
	}
	return 0;
}

static int select_on_stdin()
{
	fprintf(stdout, "%s: stdin ready for read\n", __FUNCTION__);
	char buf[1024];
	fgets(buf, sizeof(buf), stdin);
	fprintf(stdout, "%s: stdin='%s'\n", __FUNCTION__, buf);
	return 0;
}

