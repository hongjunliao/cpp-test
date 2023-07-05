/*!
 * springboot-sidecar sample c client
 * @author hongjun.liao <docici@126.com>, @date 2018/1/4
 */
#include "cpp_test.h"
#include "hp/str_dump.h"
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdlib.h> 	/* atoi */
#include <unistd.h>
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>      /* errno */

#define LISTENQ 1024		   /* for socket/listen */
#define SOCK_CLI_MAX 1024   /**/

struct sockcli {
	int fd;
	char ip[16];
	int alive;
};

struct selectctx {
	fd_set rfds;
	fd_set wfds;
	int maxfd;
};


/* MAX for client, 0 for NOT use */
static struct sockcli g_xhsdk_clis[SOCK_CLI_MAX] = { 0 };
static int xhsdkclient_add(sockcli const & cli);

static int select_on_socket_connect(selectctx & sctx, sockcli & cli);
/* rw: read/write */
static int select_on_socket_rw(selectctx & sctx);

void selectctx_init(struct selectctx * ctx, int listenfd)
{
	FD_ZERO(&ctx->rfds);
	FD_ZERO(&ctx->wfds);
	FD_SET(listenfd, &ctx->rfds);
	ctx->maxfd = listenfd;
}

static void selectctx_set_cli_fds(struct selectctx * ctx)
{
	int i = 0;
	for(; i < SOCK_CLI_MAX; ++i){
		int fd = g_xhsdk_clis[i].fd;
		if(fd > 0){
			FD_SET(fd, &ctx->rfds);
			FD_SET(fd, &ctx->wfds);
			ctx->maxfd = fd > ctx->maxfd ? fd : ctx->maxfd;
		}
	}
}

static int select_on_socket_connect(selectctx & sctx, sockcli & cli)
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

	sctx.maxfd = confd > sctx.maxfd? confd : sctx.maxfd;
	FD_SET(confd, &sctx.rfds);
	FD_SET(confd, &sctx.wfds);

	return 0;
}

static int xhsdkclient_add(sockcli const & cli)
{
	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		if(g_xhsdk_clis[i].fd == 0){
			g_xhsdk_clis[i] = cli;
			return 0;
		}
	}
	return -1;
}

static int sidecar_select_on_read(selectctx & sctx, int fd)
{
	char buf[1024];
	ssize_t r = read(fd, buf, sizeof(buf));
	if(r <=0){
		if(r < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;
		return -1;
	}
	fprintf(stdout, "%s: read done, fd=%d, buf='%s', len=%zd\n"
			, __FUNCTION__, fd, dumpstr(buf, r, 256), r);
	return r;
}

static int sidecar_select_on_write(selectctx & sctx, int fd)
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

static int select_on_socket_rw(selectctx & sctx)
{
	fd_set * rfds = &sctx.rfds, * wfds = &sctx.wfds;

	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		int& fd = g_xhsdk_clis[i].fd;

		if( fd != 0 && FD_ISSET(fd, rfds)){
			if(sidecar_select_on_read(sctx, fd) > 0)
				g_xhsdk_clis[i].alive = 1;
			else{
				fprintf(stdout, "%s: close connection, fd=%d\n"
						, __FUNCTION__, fd);
				close(fd);
				g_xhsdk_clis[i].fd = 0;
			}
		}
		if( fd != 0 && FD_ISSET(fd, wfds)){
			if(g_xhsdk_clis[i].alive){
				int r = sidecar_select_on_write(sctx, fd);
				g_xhsdk_clis[i].alive = 0;

				fprintf(stdout, "%s: close connection, fd=%d\n"
						, __FUNCTION__, fd);
				close(fd);
				g_xhsdk_clis[i].fd = 0;
			}
		}
	}
	return 0;
}

int sidecar_select_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);
	int port = 8200;

	if(argc > 1) port = atoi(argv[1]);

	int fd;
	struct sockaddr_in	servaddr = { 0 };

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "%s: socket error(socket failed)\n", __FUNCTION__);
		return -1;
	}
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "%s: setsockopt SO_REUSEADDR: %s", __FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(fd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s: socket error, errno=%d, error='%s'\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}
	if(listen(fd, LISTENQ) < 0){
		fprintf(stderr, "%s: socket error(listen failed)\n", __FUNCTION__);
		return -1;
	}

	fprintf(stdout, "%s: listening on port=%d...\n", __FUNCTION__, port);


	struct timeval timeout = {3, 0};
	for(;;){
		selectctx sctx;
		fd_set * rfds = &sctx.rfds;
		selectctx_init(&sctx, fd);
		selectctx_set_cli_fds(&sctx);

		int ret = select(sctx.maxfd + 1, &sctx.rfds, &sctx.wfds, (fd_set *)0, &timeout);
		if(ret == -1){
			fprintf(stderr, "%s: select failed\n", __FUNCTION__);
			break;
		}
		if(ret == 0){
//			fprintf(stdout, "%s: select timedout\n", __FUNCTION__);
			continue;
		}

		if (FD_ISSET(fd, rfds)) {	/* connect */
			sockcli cli = { fd, "" };
			if(select_on_socket_connect(sctx, cli) == -1)
				break;
		}
		select_on_socket_rw(sctx);/* socket I/O */
	}

	return 0;
}

static char const * help_sidecar_select()
{
	return "sidecar_select";
}

struct test_entry sidecar_select = { "sidecar_select", sidecar_select_main, help_sidecar_select };
