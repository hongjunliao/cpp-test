/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>, @date 2017/11/9
 *
 * test for fork() system call
 *
 * how to test
 *
 * client:
 * $cat file1  | nc localhost 9000 | md5sum
 *
 * server:
 * $./cpp-test-main fork 9000
 *
 */
#ifdef __GNUC__

#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdlib.h> 	/* atoi */
#include <unistd.h>     /* fork */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>       /* errno */
#include <assert.h>      /* assert */
#include "string_util.h" /* byte_to_mb_kb_str_r */

#define LISTENQ            128		     /* for socket/listen */
#define SOCK_CLI_MAX       1            /**/
#define SOCK_BUF           (1024 * 10)

struct sock_cli {
	int fd;
	char ip[16];

	char buf[SOCK_BUF];
	int len;

	int r, w;

	int stat;
};

struct SelectContext {
	fd_set rfds;
	fd_set wfds;
	int maxfd;
};

static pid_t gpid = 0;
/* MAX for client, 0 for NOT use */
static sock_cli gfds[SOCK_CLI_MAX] = { 0 };

static int sock_cli_size()
{
	int ret  = 0;
	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		if(gfds[i].fd > 0){
			++ret;
		}
	}
	return ret;
}

static int sock_cli_add(sock_cli const & cli)
{
	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		if(gfds[i].fd == 0){
			gfds[i] = cli;
			return 0;
		}
	}
	return -1;
}

static void sock_cli_remove(struct sock_cli * cli)
{
	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		if(gfds[i].fd == cli->fd){
			gfds[i].fd = 0;
			return;
		}
	}
}

static int select_on_socket_connect(SelectContext & sctx, sock_cli & cli)
{
	sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);

	int confd = accept(cli.fd, (struct sockaddr *)&clientaddr, &len);
	if(confd < 0){
		fprintf(stderr, "%s/%d: accept failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}
	cli.fd = confd;

	inet_ntop(AF_INET, &clientaddr.sin_addr, cli.ip, sizeof(cli.ip));
	fprintf(stdout, "%s/%d: connection from '%s'\n", __FUNCTION__, gpid, cli.ip);

	if(sock_cli_add(cli) != 0){
		fprintf(stderr, "%s/%d: TOO many clients, forking child ...\n", __FUNCTION__, gpid);

		pid_t cpid = fork();
		if(cpid == 0){ /* child process */

		}
		else if(cpid < 0){
			fprintf(stderr, "%s/%d: fork child process failed, errno=%d, error='%s', closing client ...\n"
					, __FUNCTION__, gpid, errno, strerror(errno));
			close(confd);
			return -2;
		}
	}

	FD_SET(confd, &sctx.rfds);
	FD_SET(confd, &sctx.wfds);
	sctx.maxfd = confd > sctx.maxfd? confd : sctx.maxfd;

	return 0;
}

static int protocol_echo_read(SelectContext & sctx, struct sock_cli * cli)
{
	if(SOCK_BUF - cli->len < 10)
		return 0; /* write op too slow */

	fd_set * rfds = &sctx.rfds;
	fd_set * wfds = &sctx.wfds;
	int fd = cli->fd;

	ssize_t ret = read(fd, cli->buf + cli->len, 10);
	if(ret <= 0){
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		fprintf(stdout, "%s/%d: read failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		close(fd);
		FD_CLR(fd, rfds);
		FD_CLR(fd, wfds);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, ret, dumpstr(cli->buf, ret, 32));

		return 0;
	}

	cli->len += ret;

	cli->r += ret;

	if(cli->stat != cli->r / (1024 * 1024 * 10)  ){
		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, ret, dumpstr(cli->buf, ret, 32));

		cli->stat = cli->r / (1024 * 1024 * 10);
	}
	return ret;
}

static int protocol_echo_write(SelectContext & sctx, struct sock_cli * cli)
{
	if(cli->len <= 0)
		return 0; /* nothing to write */

	fd_set * rfds = &sctx.rfds;
	fd_set * wfds = &sctx.wfds;
	int fd = cli->fd;

	ssize_t ret = write(fd, cli->buf, cli->len);
	if(ret <= 0){
		if(ret < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		fprintf(stdout, "%s/%d: write failed, errno=%d, error='%s'\n"
						, __FUNCTION__, gpid, errno, strerror(errno));
		close(fd);
		FD_CLR(fd, rfds);
		FD_CLR(fd, wfds);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, ret, dumpstr(cli->buf, ret, 32));

		return 0;
	}


	if(ret < cli->len){
		fprintf(stdout, "%s/%d: memmove, towrite=%d, written=%ld, left=%d\n"
				, __FUNCTION__, gpid, cli->len, ret, cli->len - (int)ret);
		memmove(cli->buf, cli->buf + ret, cli->len - ret);
	}

	cli->len -= ret;
	cli->w += ret;

	return ret;
}

static int select_on_socket_rw(SelectContext & sctx)
{
	fd_set * rfds = &sctx.rfds;
	fd_set * wfds = &sctx.rfds;

	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		int fd = gfds[i].fd;
		if(!(fd > 0))
			continue;

		if(FD_ISSET(fd, rfds)){ /* readable */
			protocol_echo_read(sctx, &gfds[i]);
		}
		if(FD_ISSET(fd, wfds)){ /* writable */
			protocol_echo_write(sctx, &gfds[i]);
		}
	}
	return 0;
}

int test_fork_call_main(int argc, char ** argv)
{
	gpid = getpid();

	fprintf(stdout, "%s/%d: build at %s %s\n", __FUNCTION__, gpid, __DATE__, __TIME__);
	int port = 80;

	if(argc > 1) port = atoi(argv[1]);

	struct sockaddr_in	servaddr = { 0 };
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	if(listenfd < 0){
		fprintf(stderr, "%s/%d: socket failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int yes = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "%s: setsockopt SO_REUSEADDR: %s", __FUNCTION__, strerror(errno));
        close(listenfd);
        return -1;
    }

	if(bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s/%d: bind error, port=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, port, errno, strerror(errno));
		return -1;
	}
	if(listen(listenfd, LISTENQ) < 0){
		fprintf(stderr, "%s/%d: listen error, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}

	fprintf(stdout, "%s/%d: listening on port=%d...\n", __FUNCTION__, gpid, port);

	SelectContext sctx;

	FD_ZERO(&sctx.rfds);
	FD_ZERO(&sctx.wfds);

	sctx.maxfd = listenfd;

	for(;;){
		FD_SET(listenfd, &sctx.rfds);
		FD_SET(listenfd, &sctx.wfds);

		int ret = select(sctx.maxfd + 1, &sctx.rfds, &sctx.wfds, (fd_set *)0, 0);
		if(ret == -1){
			fprintf(stderr, "%s/%d: select failed, errno=%d, error='%s'\n"
					, __FUNCTION__, gpid, errno, strerror(errno));
			break;
		}

		if (FD_ISSET(listenfd, &sctx.rfds)) {	/* connect */
			sock_cli cli = { listenfd };
			if(select_on_socket_connect(sctx, cli) != 0){
				close(listenfd);
				break;
			}
		}

		select_on_socket_rw(sctx);/* socket I/O */
	}

	return 0;
}

#endif /* __GNUC__ */
