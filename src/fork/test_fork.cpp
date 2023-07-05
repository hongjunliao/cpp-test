#ifdef CPP_TEST_WITH_FORK

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
 * $./cpp-test-main fork -p9000 -b1
 *
 */

#ifdef __GNUC__

#include <stdio.h>       /* setvbuf */
#include <signal.h>      /* signal */
#include <string.h> 	 /* strlen */
#include <stdlib.h> 	 /* atoi */
#include <unistd.h>      /* fork */
#include <sys/socket.h>	 /* basic socket definitions */
#include <sys/ioctl.h>   /* ioctl */
#include <netinet/in.h>	 /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	 /* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>       /* errno */
#include <assert.h>      /* assert */
#include "string_util.h" /* byte_to_mb_kb_str_r */
#include "str_dump.h"    /**/

#include <semaphore.h>   /* sem_t */
#include <fcntl.h>       /* O_CREAT */

#ifdef __linux__
#include <linux/netfilter_ipv4.h>        /* SO_ORIGINAL_DST */
#else
#warning "SO_ORIGINAL_DST needed but __linux__ NOT defined, will NOT work correctly on this platform"
#define SO_ORIGINAL_DST 0
#endif /* __linux__ */

/* hp_sig.c */
extern "C"{
void setupSignalHandlers(void);
void set_on_sig(void (*sigchld)(), void (*on_exit)(), void (*on_usr1)());
}

#define LISTENQ            128		    /* for socket/listen */
#define SOCK_CLI_MAX       2            /**/
#define SOCK_BUF           (1024 * 10)
#define PID_FILE           "/run/xhhp.pid"
#define SEM_LISTENFD       "/.xhhp"
#define MWMSG_SZ          64

struct sock_cli {
	int fd;
	char ip[16];

	char buf[SOCK_BUF];
	int len;

	int r, w;

	int stat;
};

struct evtctx {
	fd_set   rfds;
	fd_set   wfds;
	int      maxfd;
};

/* process ids of master and workers
 * if it's a worker gcpid ignored */
static pid_t gpid = 0, gcpid = 0;
static sem_t * gsemfd = 0;
static int gverbose = 0;
static int gechobytes = 10;
/* MAX for client, 0 for NOT use */
static sock_cli gfds[SOCK_CLI_MAX] = { 0 };
static int verbose = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////
static void evtctx_init(struct evtctx* ctx)
{
	if(!ctx) return;

	ctx->maxfd = 0;
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
			if(gverbose > 6)
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
			if(gverbose > 6)
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

	if(sock_cli_add(cli) != 0){
		fprintf(stderr, "%s/%d: connection from '%s', TOO many clients, closing, max=%d\n"
				, __FUNCTION__, gpid, cli.ip, SOCK_CLI_MAX);
		close(confd);
		return 0;
	}

	fprintf(stdout, "%s/%d: connection from '%s', clients=%d, max=%d\n"
			, __FUNCTION__, gpid, cli.ip, sock_cli_size(), SOCK_CLI_MAX);

	FD_SET(confd, &ctx->rfds);
	FD_SET(confd, &ctx->wfds);
	ctx->maxfd = confd > ctx->maxfd? confd : ctx->maxfd;

	return 0;
}

static int protocol_echo_read(struct evtctx * ctx, struct sock_cli * cli)
{
	int RAEDN = gechobytes;

	if(SOCK_BUF - cli->len < RAEDN)
		return 0; /* write op too slow */

	int fd = cli->fd;

	ssize_t n = read(fd, cli->buf + cli->len, RAEDN);
	if(n <= 0){
		if(gverbose > 3)
			fprintf(stdout, "%s/%d: read failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));

		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		close(fd);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, read=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, n, dumpstr(cli->buf, cli->len, 32));

		return 0;
	}

	cli->len += n;

	cli->r += n;

	if(verbose > 3){
		if(cli->stat != cli->r / (1024 * 1024 * 10)  ){
			char bufr[64], bufw[64];
			byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
			byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

			fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, read=%ld, buff='%s'\n"
				, __FUNCTION__, gpid, fd, cli->r, bufr
				, cli->w, bufw
				, cli->r - cli->w, n, dumpstr(cli->buf, cli->len, 32));

			cli->stat = cli->r / (1024 * 1024 * 10);
		}
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
		if(gverbose > 3)
			fprintf(stdout, "%s/%d: write failed, errno=%d, error='%s'\n"
						, __FUNCTION__, gpid, errno, strerror(errno));

		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;

		close(fd);

		sock_cli_remove(cli);

		char bufr[64], bufw[64];
		byte_to_mb_kb_str_r(cli->r, "%-.2f %cB", bufr);
		byte_to_mb_kb_str_r(cli->w, "%-.2f %cB", bufw);

		fprintf(stdout, "%s/%d: fd=%d, totalr=%d/%s, totalw=%d/%s, left=%d, write=%ld, buff='%s'\n"
			, __FUNCTION__, gpid, fd, cli->r, bufr
			, cli->w, bufw
			, cli->r - cli->w, n, dumpstr(cli->buf, cli->len, 32));

		return 0;
	}


	if(n < cli->len){
		if(verbose > 3)
			fprintf(stdout, "%s/%d: memmove, towrite=%d, written=%ld, left=%d\n"
				, __FUNCTION__, gpid, cli->len, n, cli->len - (int)n);
		memmove(cli->buf, cli->buf + n, cli->len - n);
	}

	cli->len -= n;
	cli->w += n;

	return n;
}

static int select_on_cli_rw(struct evtctx * ctx)
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

static int select_on_mw_w(struct evtctx * ctx, int fd)
{
	if(!(ctx && fd > 0)) return -1;

	char buf[MWMSG_SZ];
	int m = sprintf(buf, "%d,%d", gpid, sock_cli_size());
	assert(m > 0);

	int n = write(fd, buf, strlen(buf));

	if(n <= 0){
		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;
		return -1;
	}

	if(!(n == m))
		return -1;

	return 0;
}

static int select_on_mw_r(struct evtctx * ctx, int fd)
{
	static size_t count = 0;

	if(!(ctx && fd > 0)) return -1;

	char buf[MWMSG_SZ];
	int n = read(fd, buf, sizeof(buf));

	if(n <= 0){
		if(n < 0 && (errno == EAGAIN || errno == EINTR))
			return 0;
		return -1;
	}

	pid_t pid = 0;
	int clisz = 0;
	n = sscanf(buf, "%d,%d", &pid, &clisz);

	if(n != 2){
		fprintf(stderr, "%s/%d: sscanf failed, errno=%d, error='%s'\n"
			, __FUNCTION__, gpid, errno, strerror(errno));

		return -1;
	}

	++count;

	if(count % 200000 == 0)
		fprintf(stderr, "%s/%d: report from worker: pid=%d, client_size=%d, max=%d, report=%zu\n"
				, __FUNCTION__, gpid, pid, clisz, SOCK_CLI_MAX, count);
	return 0;
}

static int save_pidfile(FILE * f)
{
	if(!f) return -1;

	int r = fprintf(f, "%d\n", gpid);
	if(r <= 0){
		fprintf(stderr, "%s/%d: save to pidfile '%s' failed, pid=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, PID_FILE, gpid, errno, strerror(errno));
		return -1;
	}
	fflush(f);
	fclose(f);
	return 0;
}

static int hdl_sig_stop()
{
	FILE * f = fopen(PID_FILE, "r");
	if(!f) {
		fprintf(stderr, "%s/%d: fopen pid file '%s' for read failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, PID_FILE, errno, strerror(errno));
		return -1;
	}

	int pids[128];
	int i = 0;

	char bufline[1024];
	for(; fgets(bufline, sizeof(bufline), f); ++i){
	     int pid = atoi(bufline);
#ifndef NDEBUG
		fprintf(stdout, "%s/%d: pid=%d\n"
				, __FUNCTION__, gpid, pid);
#endif /* NDEBUG */
		 pids[i] = pid;
		 if(pid > 1){
			 int r = kill(pid, SIGINT);
			 if(r == -1){
				 fprintf(stderr, "%s/%d: kill pid=%d, failed, signal='%s', errno=%d, error='%s'\n"
						 , __FUNCTION__, gpid, pid, "SIGINT", errno, strerror(errno));
			 }
		 }
	}
#ifndef NDEBUG
		fprintf(stdout, "%s/%d: signal sent, pid_count=%d\n"
				, __FUNCTION__, gpid, i);
#endif /* NDEBUG */
	return 0;
}

static int test_fork_call_test_main()
{
	char buf[MWMSG_SZ]="235,1";
	pid_t pid = 0;
	int clisz = 0;
	int n = sscanf(buf, "%d,%d", &pid, &clisz);

	fprintf(stderr, "%s/%d: sscanf buf='%s', return=%d, pid=%d, client_size=%d, max=%d\n"
				, __FUNCTION__, gpid, buf, n, pid, clisz, SOCK_CLI_MAX);
	assert(n == 2 && clisz == 1 && pid == 235);
	return 0;
}

static void on_sigchld()
{
	sem_post(gsemfd);
}

int test_fork_call_main(int argc, char ** argv)
{
	test_fork_call_test_main();

	setvbuf(stdout, 0, _IONBF, 0);

	/* see redis/server.c/initServer */
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
	setupSignalHandlers();
	set_on_sig(on_sigchld, 0, 0);

	gpid = getpid();

	fprintf(stdout, "%s/%d: build at %s %s\n", __FUNCTION__, gpid, __DATE__, __TIME__);
	int port = 80;
	int sig_stop = 0, sig_reload = 0;

	for(int i = 0; i < argc; ++i){
		if(strncmp(argv[i], "-p", 2) == 0)
			port = atoi(argv[i] + 2);
		else if(strncmp(argv[i], "-v", 2) == 0)
			gverbose = atoi(argv[i] + 2);
		else if(strncmp(argv[i], "-b", 2) == 0)
			gechobytes = atoi(argv[i] + 2);
		else if(strncmp(argv[i], "-s", 2) == 0) {
			if(strncmp(argv[i] + 2, "stop", 4) == 0) sig_stop = 1;
			else if(strncmp(argv[i] + 2, "reload", 5) == 0) sig_reload = 1;
			else
				fprintf(stderr, "%s/%d: unkown signal: '%s'\n"
								, __FUNCTION__, gpid, argv[i] + 2);
		}
	}

#ifndef NDEBUG
		fprintf(stdout, "%s/%d: -p/port=%d, -b/echo_bytes=%d, -sstop/stop=%d, -sreload/reload=%d, -v/verbose=%d\n"
				, __FUNCTION__, gpid, port, gechobytes, sig_stop, sig_reload, gverbose);
#endif /* NDEBUG */

	if(sig_stop){
		fprintf(stdout, "%s/%d: stopping\n", __FUNCTION__, gpid);
		hdl_sig_stop();
		return 0;
	}
	if(sig_reload){
		fprintf(stdout, "%s/%d: reloading\n", __FUNCTION__, gpid);
	}

	FILE * pidfile = 0;
	pidfile = fopen(PID_FILE, "w");
	if(!pidfile) {
		fprintf(stderr, "%s/%d: fopen pidfile '%s' for write failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, PID_FILE, errno, strerror(errno));
		return -1;
	}

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

	{
		unsigned long sockopt = 1;
		if(ioctl(listenfd, FIONBIO, &sockopt) < 0)
			fprintf(stdout, "%s/%d: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, gpid, listenfd);
	}

	int yes = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        fprintf(stderr, "%s: setsockopt SO_REUSEADDR failed: errno=%d, error='%s'\n"
        		, __FUNCTION__, errno, strerror(errno));
        close(listenfd);
        return -1;
    }

	if(bind(listenfd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s/%d: bind failed, port=%d, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, port, errno, strerror(errno));
		return -1;
	}
	if(listen(listenfd, LISTENQ) < 0){
		fprintf(stderr, "%s/%d: listen error, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}

	sem_unlink(SEM_LISTENFD);

	int mwfd[2];
	if(socketpair(AF_LOCAL, SOCK_DGRAM, 0, mwfd) < 0){
		fprintf(stderr, "%s/%d: socketpair failed, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, errno, strerror(errno));
		return -1;
	}
	/* 0 for read, 1 for write */
	int mwrfd = mwfd[0], mwwfd = mwfd[1];
	{
		unsigned long sockopt = 1;
		if(ioctl(mwrfd, FIONBIO, &sockopt) < 0)
			fprintf(stdout, "%s/%d: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, gpid, mwrfd);
	}{
		unsigned long sockopt = 1;
		if(ioctl(mwwfd, FIONBIO, &sockopt) < 0)
			fprintf(stdout, "%s/%d: ioctl(FIONBIO) failed for fd=%d\n", __FUNCTION__, gpid, mwwfd);
	}

	pid_t pid = fork();
	if(pid < 0){
		fprintf(stdout, "%s/%d: fork failed, exit, errno=%d, error='%s'\n"
				, __FUNCTION__, getpid(), errno, strerror(errno));
		return -1;
	}
	else if (pid == 0) { /* child process */
		gpid = getpid();
		gcpid = 0;
	}
	else
		gcpid = pid;

	save_pidfile(pidfile);

	gsemfd = sem_open(SEM_LISTENFD, O_CREAT, S_IRWXU, 1);
	if(gsemfd == SEM_FAILED){
		fprintf(stderr, "%s/%d: sem_open('%s') failed, sem=%p, errno=%d, error='%s'\n"
				, __FUNCTION__, gpid, SEM_LISTENFD, gsemfd, errno, strerror(errno));
		return -1;
	}

	fprintf(stdout, "%s/%d(%d): listening on port=%d...\n", __FUNCTION__, gpid, gcpid, port);

	struct evtctx sctx, * ctx = &sctx;
	for(;;){
		evtctx_init(ctx);

		if(gcpid != 0) { /* master process, always readable */
			FD_SET(mwrfd, &ctx->rfds);
			if(mwrfd > ctx->maxfd)
				ctx->maxfd = mwrfd;
		}

		int gotfd = 0;
		int clisz = sock_cli_size();

		/* if too much clients stop adding to select
		 * NOTE: new connect requests dosen't refused by listenfd immediately
		 * instead they are put to 'backlog', see listen() and LISTENQ */
		if(clisz < SOCK_CLI_MAX * 0.8){
			int result = sem_trywait(gsemfd);
			if(result == 0){
				fprintf(stdout, "%s/%d(%d): got lock!________________________________\n"
						, __FUNCTION__, gpid, gcpid);

				gotfd = 1;

				FD_SET(listenfd, &ctx->rfds);
				FD_SET(listenfd, &ctx->wfds);
				ctx->maxfd = listenfd;
			}
		}
		else {
//			fprintf(stdout, "%s/%d: clients almost full, stop receive clients, clients=%d, max=%d\n"
//				, __FUNCTION__, gpid, clisz, SOCK_CLI_MAX);

			/* report to master process that we need more worker process */
			if(gcpid == 0){
				FD_SET(mwwfd, &ctx->wfds);
				if(mwwfd > ctx->maxfd)
					ctx->maxfd = mwwfd;
			}
		}

		for(int i = 0; i < SOCK_CLI_MAX; ++i){
			int fd = gfds[i].fd;
			if(fd > 0){
				FD_SET(fd, &ctx->rfds);
				FD_SET(fd, &ctx->wfds);

				if(fd > ctx->maxfd)
					ctx->maxfd = fd;
			}
		}

		if(ctx->maxfd == 0){
			sleep(1);
			continue;
		}

		struct timeval timeout = { 1, 0 };
		int ret = select(ctx->maxfd + 1, &ctx->rfds, &ctx->wfds, (fd_set *)0, &timeout);

		if(gotfd){
			fprintf(stdout, "%s/%d(%d): release lock________________________________\n"
								, __FUNCTION__, gpid, gcpid);
			gotfd = 0;
			sem_post(gsemfd);

		}
		for(size_t i = 0; i < 1000000000; ++i)
			;

		if(ret == -1){
			fprintf(stderr, "%s/%d: select failed, errno=%d, error='%s'\n"
					, __FUNCTION__, gpid, errno, strerror(errno));

			if(errno == EINTR || errno == EAGAIN)
				continue;

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

		select_on_cli_rw(ctx);/* clients socket I/O */

		/* worker process report to master */
		if (gcpid == 0 && FD_ISSET(mwwfd, &ctx->wfds)){
			int r = select_on_mw_w(ctx, mwwfd);
//			fprintf(stdout, "%s/%d(%d): select_on_mw_w, return=%d\n"
//					, __FUNCTION__, gpid, gcpid, r);
		}

		/* master process receive reports from worker */
		if (gcpid != 0 && FD_ISSET(mwrfd, &ctx->rfds))
			select_on_mw_r(ctx, mwrfd);
	}

	return 0;
}

#endif /* __GNUC__ */

#endif

