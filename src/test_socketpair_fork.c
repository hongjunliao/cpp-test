/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2018/8/16
 *
 * https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/socketpair.c
 * */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>


int test_socketpair_fork_main(int argc, char ** argv)
{
	int sv[2]; /* the pair of socket descriptors */
	char buf; /* for data exchange between processes */

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
		perror("socketpair");
		exit(EXIT_FAILURE);
	}

	if (!fork()) {  /* child */
		read(sv[1], &buf, 1);
		printf("child: read '%c'\n", buf);
		buf = toupper(buf);  /* make it uppercase */
		write(sv[1], &buf, 1);
		printf("child: sent '%c'\n", buf);

	} else { /* parent */
		write(sv[0], "b", 1);
		printf("parent: sent 'b'\n");
		read(sv[0], &buf, 1);
		printf("parent: read '%c'\n", buf);
		wait(NULL); /* wait for child to die */
	}

	return 0;
}
