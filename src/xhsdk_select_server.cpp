/*!
 * xhsdk_select_server
 * @author hongjun.liao <docici@126.com>, @date 2017/8/23
 */
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

/* from xhsdk */
//#include "D:/jun/sdk-windows/libprotocol/protocol.h" /* PROTOCOL_MAGIC */
#define PROTOCOL_MAGIC 							    0x11ff33ff
#define LOGIN1_RESP				            		1
#define LOGIN2_RESP					            	3

#define LISTENQ 1024		   /* for socket/listen */
#define XHSDKCLIENT_MAX 1024   /**/

struct XhSDKClient {
	int fd;
	char ip[16];
};

struct SelectContext {
	fd_set rfds;
	int maxfd;
};

/* MAX for client, 0 for NOT use */
static XhSDKClient g_xhsdk_clis[XHSDKCLIENT_MAX] = { 0 };
static int xhsdkclient_add(XhSDKClient const & cli);

static int select_on_socket_connect(SelectContext & sctx, XhSDKClient & cli);
static int select_on_stdin();
/* rw: read/write */
static int select_on_socket_rw(SelectContext & sctx);

int xhsdk_select_server_main(int argc, char ** argv)
{
	printf("%s: unsigned short=%zu, unsigned long=%zu\n", __FUNCTION__, sizeof(unsigned short),
			sizeof(unsigned long));

	fprintf(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);
	int port = 80;

	if(argc > 1) port = atoi(argv[1]);

	int fd;
	int stdinfd = fileno(stdin);
	struct sockaddr_in	servaddr = { 0 };

	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "%s: socket error(socket failed)\n", __FUNCTION__);
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

	SelectContext sctx;
	fd_set & rfds = sctx.rfds;

	FD_ZERO(&rfds);
	FD_SET(stdinfd, &rfds);
	FD_SET(fd, &rfds);
	sctx.maxfd = fd > stdinfd ? fd : stdinfd;

	struct timeval timeout = {3, 0};
	for(;;){
		FD_ZERO(&rfds);
		FD_SET(sctx.maxfd, &sctx.rfds);
		int ret = select(sctx.maxfd + 1, &sctx.rfds, (fd_set *)0, (fd_set *)0, &timeout);
		if(ret == -1){
			fprintf(stderr, "%s: select failed\n", __FUNCTION__);
			break;
		}
		if(ret == 0){
//			fprintf(stdout, "%s: select timedout\n", __FUNCTION__);
			continue;
		}

		if (FD_ISSET(fd, &rfds)) {	/* connect */
			XhSDKClient cli = { fd, "" };
			if(select_on_socket_connect(sctx, cli) == -1)
				break;
		}

		select_on_socket_rw(sctx);/* socket I/O */

		if(FD_ISSET(stdinfd, &rfds)){ /* stdin */
			select_on_stdin();
		}
	}

	return 0;
}

static int select_on_socket_connect(SelectContext & sctx, XhSDKClient & cli)
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

static int protocol_echo(SelectContext & sctx, int fd)
{
	fd_set & rfds = sctx.rfds;

	char buf[1024];
	ssize_t ret = read(fd, buf, sizeof(buf));
	if(ret == -1 || ret == 0){
		if(ret == -1)
			fprintf(stderr, "%s: socket error(read failed)\n", __FUNCTION__);
		else
			fprintf(stdout, "%s: client closed\n", __FUNCTION__);

		FD_CLR(fd, &rfds);
		close(fd);
		fd = 0;		/* set this client to NOT used */
		return -1;
	}
	buf[ret] = '\0';	/* required */
	fprintf(stdout, "%s: fd=%d, len=%ld, buff='%s'\n", __FUNCTION__, fd, ret, buf);

	char * t = strchr(buf, ':');
	if(t) *t = '\0';
	sprintf(buf, "%s: hello client", t? buf : "(nil)");

	ret = write(fd, buf, strlen(buf));
	if(ret <= 0){
		fprintf(stderr, "%s: write failed, closing\n", __FUNCTION__);

		FD_CLR(fd, &rfds);
		close(fd);
		fd = 0;
	}
	return 0;
}

static void packet_xhsdkclient_login_resp(char * buf, int & len)
{
	/* @author hongjun.liao <docici@126.com>
	 * NOTE: magic, cmd, and clen MUST be the same width of in sdk_windows */
	int magic = PROTOCOL_MAGIC;
	unsigned short cmd = LOGIN1_RESP;
	char err = 0;

	char const * cmdstr = "0192.168.1.101:8080,192.168.1.102:8089,192.168.1.102:8082";
	int clen = strlen(cmdstr);

	len = 0;
	memcpy(buf + len, &magic, sizeof(magic));
	len += sizeof(magic);

	memcpy(buf + len, &cmd, sizeof(cmd));
	len += sizeof(cmd);

	memcpy(buf + len, &clen, sizeof(clen));
	len += sizeof(clen);

	memcpy(buf + len, &err, sizeof(err));
	len += sizeof(err);

	strcpy(buf + len, cmdstr);
	len += clen;
}

static int protocol_xhsdkclient_login(SelectContext & sctx, int fd)
{
	fd_set & rfds = sctx.rfds;

	char buf[1024];
	ssize_t ret = read(fd, buf, sizeof(buf));
	if(ret == -1 || ret == 0){
		if(ret == -1)
			fprintf(stderr, "%s: socket error(read failed)\n", __FUNCTION__);
		else
			fprintf(stdout, "%s: client closed\n", __FUNCTION__);

		FD_CLR(fd, &rfds);
		close(fd);
		fd = 0;		/* set this client to NOT used */
		return -1;
	}
	buf[ret] = '\0';	/* required */
	fprintf(stdout, "%s: fd=%d, len=%ld, buff='%s'\n", __FUNCTION__, fd, ret, buf);

	int packet_len = 0;
	packet_xhsdkclient_login_resp(buf, packet_len);

	ret = write(fd, buf, packet_len);
	if(ret <= 0){
		fprintf(stderr, "%s: write failed, closing\n", __FUNCTION__);

		FD_CLR(fd, &rfds);
		close(fd);
		fd = 0;
	}
	fprintf(stdout, "%s: login_resp, fd=%d, len=%ld, buff='%s'\n", __FUNCTION__, fd, ret, buf);

	return 0;
}

static int select_on_socket_rw(SelectContext & sctx)
{
	fd_set & rfds = sctx.rfds;

	for(int i = 0; i < XHSDKCLIENT_MAX; ++i){
		int& fd = g_xhsdk_clis[i].fd;
		if( fd != 0 && FD_ISSET(fd, &rfds)){
			protocol_echo(sctx, fd);
//			protocol_xhsdkclient_login(sctx, fd);
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

