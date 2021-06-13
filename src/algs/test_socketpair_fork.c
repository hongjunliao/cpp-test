/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2018/8/16
 *
 * https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/socketpair.c
 * */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

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

#define BUF_SIZE 30

int test_socketpair_fork2_main(int argc, char ** argv)
{
    int s[2];
    int w,r;
    char const * ping = "hello";
    char const * pong = "hello!";

    char * buf = (char*)calloc(1 , BUF_SIZE);
    pid_t pid;

    if( socketpair(AF_UNIX,SOCK_STREAM,0,s) == -1 ){
            printf("create unnamed socket pair failed:%s\n",strerror(errno) );
            exit(-1);
    }

    /***********Test : fork but don't close any fd in neither parent nor child process***********/
    if( ( pid = fork() ) > 0 ){
            close(s[1]);
            if( ( w = write(s[0] , ping , strlen(ping)) ) == -1 ){
                    printf("%d: socket error:%s\n", getpid(), strerror(errno));
                    exit(-1);
            }
            printf("%d=>%d: %s\n", getpid(), pid, ping);

            if( (r = read(s[0], buf , BUF_SIZE )) == -1){
                    printf("%d: read from socket error:%s\n",getpid() , strerror(errno) );
                    exit(-1);
            }
            printf("%d<=: %s\n", getpid(), buf);
    }else if(pid == 0){
            close(s[0]);
            if( (r = read(s[1], buf , BUF_SIZE )) == -1){
                    printf("%d: read from socket error:%s\n", getpid() , strerror(errno) );
                    exit(-1);
            }
            printf("%d<=: %s\n", getpid(), buf);

            if( ( w = write(s[1] , pong , strlen(pong) ) ) == -1 ){
                    printf("%d: Write socket error:%s\n", getpid(), strerror(errno));
                    exit(-1);
            }
            printf("%d=>%d: %s\n", getpid(), pid, pong);
    }else{
            printf("Fork failed:%s\n",strerror(errno));
            exit(-1);
    }
    exit(0);
}
