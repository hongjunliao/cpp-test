/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/7/3
 *
 * */
#include <stdio.h>
#include <unistd.h>
#include <string.h>		/* strlen */
#include <sys/socket.h>	/* basic socket definitions */
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>	/* inet_ntop */
#include <sys/select.h>
#include <time.h>
#include <errno.h>      /* errno */

int udp_echo_cli_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: udp_echo_cli, build at %s %s\n", __FUNCTION__, __DATE__, __TIME__);

	int fd;
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "%s: socket failed, errno=%d, error='%s'\n", __FUNCTION__, errno, strerror(errno));
		return -1;
	}


	char const * ip = "127.0.0.1";
	int port = 9000;
	struct sockaddr_in servaddr = { 0 }, peer = { 0 };

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &servaddr.sin_addr);

	size_t t = 0;
	char buf[512];
	socklen_t peer_len = 0;
	for(;;){
		sprintf(buf, "%zu: hello, xhsdkserver", t++);

		fprintf(stdout, "%s: send '%s' to %s:%d\n", __FUNCTION__, buf, ip , port);

		ssize_t ret = sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(ret == -1){
			fprintf(stderr, "%s: write failed, buff='%s'\n", __FUNCTION__, buf);
			break;
		}
		ret = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&peer, &peer_len);
		if(ret != -1){
			buf[ret] = '\0'; /* required */
			fprintf(stdout, "%s: from server, len=%ld, buff='%s'\n", __FUNCTION__, ret, buf);
		}
		sleep(3);
	}
	return 0;
}
