/*!
 * tcp_echo_cli
 * @author hongjun.liao <docici@126.com>, @date 2017/8/23
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>		/* strlen */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>      /* errno */

int tcp_echo_cli_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);

	int fd;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "%s: socket failed, errno=%d, error='%s'\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}


	char const * ip = "127.0.0.1";
	int port = 9000;
	struct sockaddr_in	cliaddr = { 0 }, servaddr = { 0 };

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(0);
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &servaddr.sin_addr);

	if(bind(fd, (sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
		fprintf(stderr, "%s: bind failed, errno=%d, error='%s'\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}
	if(connect(fd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		fprintf(stderr, "%s: connect failed, server='%s:%d', errno=%d, error='%s'\n"
				, __FUNCTION__, ip, port, errno, strerror(errno));
		return -1;
	}

	size_t t = 0;
	char buf[512];
	for(;;){
		sprintf(buf, "%zu: hello, xhsdkserver", t++);
		ssize_t ret = write(fd, buf, strlen(buf));
		if(ret == -1){
			fprintf(stderr, "%s: write failed, buff='%s'\n", __FUNCTION__, buf);
			break;
		}
		ret = read(fd, buf, sizeof(buf));
		if(ret != -1){
			buf[ret] = '\0'; /* required */
			fprintf(stdout, "%s: from server, len=%ld, buff='%s'\n", __FUNCTION__, ret, buf);
		}
		sleep(3);
	}
	return 0;
}
