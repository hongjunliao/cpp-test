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

#include <stdio.h>  /* setvbuf */
#include <signal.h> /* signal */
#include <string.h> 	/* strlen */
#include <stdlib.h> 	/* atoi */
#include <unistd.h>     /* fork */
#include <sys/socket.h>	/* basic socket definitions */
#include <sys/ioctl.h>  /* ioctl */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>       /* errno */
#include <assert.h>      /* assert */
#include "string_util.h" /* byte_to_mb_kb_str_r */

#ifdef __linux__
#include <linux/netfilter_ipv4.h>   /* SO_ORIGINAL_DST */
#else
#warning "SO_ORIGINAL_DST needed but __linux__ NOT defined, will NOT work correctly on this platform"
#define SO_ORIGINAL_DST 0
#endif /* __linux__ */


#define LISTENQ            128		     /* for socket/listen */
#define SOCK_CLI_MAX       2            /**/
#define SOCK_BUF           (1024 * 10)

struct sock_cli {
	int fd;
	char ip[16];

	char buf[SOCK_BUF];
	int len;

	int r, w;

	int stat;
};

struct evtctx {
	fd_set rfds;
	fd_set wfds;
	int maxfd;
};

static pid_t gpid = 0;
/* MAX for client, 0 for NOT use */
static sock_cli gfds[SOCK_CLI_MAX] = { 0 };


///////////////////////////////////////////////////////////////////////////////////////////////////////
static void evtctx_init(struct evtctx* ctx)
{
	if(!ctx) return;

	FD_ZERO(&ctx->rfds);
	FD_ZERO(&ctx->wfds);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

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
			fprintf(stdout, "%s/%d: client added, fd=%d, left=%d\n", __FUNCTION__, gpid, cli.fd, sock_cli_size());
			return 0;
		}
	}
	return -1;
}

static void sock_cli_remove(struct sock_cli * cli)
{
	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		if(gfds[i].fd > 0 && gfds[i].fd == cli->fd){
			gfds[i].fd = 0;
			fprintf(stdout, "%s/%d: client removed, fd=%d, left=%d\n", __FUNCTION__, gpid, cli->fd, sock_cli_size());
			return;
		}
	}
}

static int select_on_socket_connect(struct evtctx * ctx, sock_cli & cli)
{
	sockaddr_in clientaddr = { 0 };
	socklen_t len = sizeof(clientaddr);

	int confd = accept(cli.fd, (struct sockaddr *)&clientaddr, &len);
	if(confd < 0 ){
		if(errno == EINTR || errno == EAGAIN)
			return 0;

		fprintf(stderr, "%s/%d: accept failed, errno=%d, error='%s'\n", __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}

	unsigned long sockopt = 1;
	if(ioctl(confd, FIONBIO, &sockopt) < 0)
		fprintf(stdout, "%s/%d: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, gpid, confd);

	cli.fd = confd;

	inet_ntop(AF_INET, &clientaddr.sin_addr, cli.ip, sizeof(cli.ip));
	fprintf(stdout, "%s/%d: connection from '%s'\n", __FUNCTION__, gpid, cli.ip);

	if(sock_cli_add(cli) != 0){
		fprintf(stderr, "%s/%d: TOO many clients, closing ...\n", __FUNCTION__, gpid);
		close(confd);
		return 0;
	}

	FD_SET(confd, &ctx->rfds);
	FD_SET(confd, &ctx->wfds);
	ctx->maxfd = confd > ctx->maxfd? confd : ctx->maxfd;

	return 0;
}

static int protocol_echo_read(struct evtctx * ctx, struct sock_cli * cli)
{
	if(SOCK_BUF - cli->len < 10)
		return 0; /* write op too slow */

	int fd = cli->fd;

	ssize_t n = read(fd, cli->buf + cli->len, 10);
	if(n <= 0){
		fprintf(stdout, "%s/%d: read failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));

		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		close(fd);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, n, dumpstr(cli->buf, n, 32));

		return 0;
	}

	cli->len += n;

	cli->r += n;

	if(cli->stat != cli->r / (1024 * 1024 * 10)  ){
		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, n, dumpstr(cli->buf, n, 32));

		cli->stat = cli->r / (1024 * 1024 * 10);
	}
	return n;
}

static int protocol_echo_write(struct evtctx * ctx, struct sock_cli * cli)
{
	if(cli->len <= 0)
		return 0; /* nothing to write */

	int fd = cli->fd;

	ssize_t n = write(fd, cli->buf, cli->len);
	if(n <= 0){
		fprintf(stdout, "%s/%d: write failed, errno=%d, error='%s'\n"
						, __FUNCTION__, gpid, errno, strerror(errno));

		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		close(fd);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, len=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, n, dumpstr(cli->buf, n, 32));

		return 0;
	}


	if(n < cli->len){
		fprintf(stdout, "%s/%d: memmove, towrite=%d, written=%ld, left=%d\n"
				, __FUNCTION__, gpid, cli->len, n, cli->len - (int)n);
		memmove(cli->buf, cli->buf + n, cli->len - n);
	}

	cli->len -= n;
	cli->w += n;

	return n;
}

static int select_on_socket_rw(struct evtctx * ctx)
{
	fd_set * rfds = &ctx->rfds;
	fd_set * wfds = &ctx->wfds;

	for(int i = 0; i < SOCK_CLI_MAX; ++i){
		int fd = gfds[i].fd;
		if(!(fd > 0))
			continue;

		if(FD_ISSET(fd, rfds)){ /* readable */
			protocol_echo_read(ctx, &gfds[i]);
		}
		if(FD_ISSET(fd, wfds)){ /* writable */
			protocol_echo_write(ctx, &gfds[i]);
		}
	}
	return 0;
}

int test_fork_call_main(int argc, char ** argv)
{
	setvbuf(stdout, 0, _IONBF, 0);
	/* see redis/server.c/initServer */
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

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

	unsigned long sockopt = 1;
	if(ioctl(listenfd, FIONBIO, &sockopt) < 0)
		fprintf(stdout, "%s/%d: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, gpid, listenfd);

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

	struct evtctx sctx, * ctx = &sctx;

	time_t startt = time(0);
	for(;;){
		evtctx_init(ctx);

		FD_SET(listenfd, &ctx->rfds);
		FD_SET(listenfd, &ctx->wfds);
		ctx->maxfd = listenfd;

		for(int i = 0; i < SOCK_CLI_MAX; ++i){
			int fd = gfds[i].fd;
			if(fd > 0){
				FD_SET(fd, &ctx->rfds);
				FD_SET(fd, &ctx->wfds);

				if(fd > ctx->maxfd)
					ctx->maxfd = fd;
			}
		}

		time_t now = time(0);
		if(now - startt > 5){
			fprintf(stdout, "%s/%d: select ...\n", __FUNCTION__, gpid);
			startt = now;
		}

		int ret = select(ctx->maxfd + 1, &ctx->rfds, &ctx->wfds, (fd_set *)0, 0);
		if(ret == -1){
			fprintf(stderr, "%s/%d: select failed, errno=%d, error='%s'\n"
					, __FUNCTION__, gpid, errno, strerror(errno));
			break;
		}

		if (FD_ISSET(listenfd, &ctx->rfds)) {	/* connect */
			sock_cli cli = { listenfd };
			if(select_on_socket_connect(ctx, cli) != 0){
				close(listenfd);
				break;
			}
			/* add new client to event system first,
			 * do I/O later */
			continue;
		}

		select_on_socket_rw(ctx);/* socket I/O */
	}

	return 0;
}

#endif /* __GNUC__ */
