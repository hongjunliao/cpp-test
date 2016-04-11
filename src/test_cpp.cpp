/*
 * test_cpp.cpp
 *
 *  Created on: Jan 5, 2016
 *      Author: root
 */
#include </usr/include/google/protobuf/message.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdarg.h>	/*va_start*/
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <syslog.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

//#include <sys/sem.h>
#include <semaphore.h>
#include "bd_test.h"

struct student{
	int age;
	char * name;

};
static student s = {
	.age = 10,
	.name = "jack"
};

/*Linux 程序设计第4版 13*/
int test_pipe_1(int argc, char ** argv);
int test_pipe_2(int argc, char ** argv);
int test_pipe_3(int argc, char ** argv);
int test_pipe_4(int argc, char ** argv);
int test_pipe_5(int argc, char ** argv);

/*Linux 程序设计第4版 13.6*/
char const * pipe_file = "/tmp/my_fifo";
int test_pipe_server(int argc, char ** argv);
int test_pipe_client(int argc, char ** argv);
/*高级主题, 使用FIFO的客户/服务器应用程序*/
struct upper_context{
	pid_t client_pid;
	char buffer[PIPE_BUF - 20];
};

char const * pipe_file_2 = "/tmp/my_fifo_2";
char const * fdp_make(char const * format, ...);
int test_pipe_server_2(int argc, char ** argv);
int test_pipe_client_2(int argc, char ** argv);
int test_dup_dup2(int argc, char ** argv);

/*14.1 信号量, XSI 版本: sys/sem.h; POSIX semaphore.h*/
int test_sem(int argc, char ** argv);

/*共享内存*/
static key_t shm_key = 400;
int test_shm_server(int argc, char ** argv);
int test_shm_client(int argc, char ** argv);
static char const * ARGS = "<help|<cmd>|server|server2|shm-server|dup>";
int test_cpp_main(int argc, char ** argv)
{
	if(argc < 2 || argc > 1 && 0 == strcmp(argv[1], "help")){
		printf("%s %s %s\n", argv[0], TEST_CPP,  ARGS);
		return 0;
	}
	/*测试gobject*/
//	printf("student s: age = %d, name = %s\n", s.age, s.name);

	char const * cmd = argc > 1? argv[1] : "ls";

	//	printf("test function system cmd = %s\n", cmd);
//	system(cmd);
//	printf("----------Done\n");

//	printf("test function exec* cmd = %s\n", cmd);
//	execl(cmd, "", 0);
//	printf("----------Done\n");

	/*管道基本测试*/
//	test_pipe_1(argc, argv);
//	test_pipe_2(argc, argv);
//	test_pipe_3(argc, argv);
//	test_pipe_4(argc, argv);
//	test_pipe_5(argc, argv);

	/*测试管道, C/S结构*/
/*
	if(0 == strcmp(argv[1], "server"))
		test_pipe_server(argc, argv);
	if(0 == strcmp(argv[1], "client"))
			test_pipe_client(argc, argv);
*/
	/*测试管道, C/S结构*/
/*
	if(0 == strcmp(argv[1], "server2"))
		test_pipe_server_2(argc, argv);
	if(0 == strcmp(argv[1], "client2"))
		test_pipe_client_2(argc, argv);
*/

	/*测试共享内存, C/S结构*/
	if(0 == strcmp(argv[1], "shm-server"))
		test_shm_server(argc, argv);
	if(0 == strcmp(argv[1], "shm-client"))
			test_shm_client(argc, argv);

	/*测试dup, dup2函数*/
	if(0 == strcmp(argv[1], "dup"))
		test_dup_dup2(argc, argv);
	return 0;
}

int test_sem(int argc, char ** argv)
{
	return 0;
}

#include <sys/shm.h>
int test_shm_server(int argc, char ** argv)
{
	int shmid = shmget(shm_key, sizeof(upper_context), 0666 | IPC_CREAT);

	if(shmid == -1){ return -1; }
	void * shmaddr = shmat(shmid, 0, 0);
	if(!shmaddr || shmaddr == (void *)-1){ return -1; }
	for(;;){
		upper_context * uc = (upper_context *) shmaddr;
		if(uc->client_pid != 0){
			if(0 == strncmp(uc->buffer, "end", 3))
				break;
			printf("%s: buffer = %s\n", __FUNCTION__, uc->buffer);
			uc->client_pid = 0;
			sleep(1);
		}
	}
	int r = shmdt(shmaddr);
	if(r == -1){ return -1; }
	r = shmctl(shmid, IPC_RMID, 0);
	if(r == -1){ return -1; }
	return 0;
}

int test_shm_client(int argc, char ** argv)
{
	int shmid = shmget(shm_key, sizeof(upper_context), 0666 | IPC_CREAT);

	if(shmid == -1){ return -1; }
	void * shmaddr = shmat(shmid, 0, 0);
	if(!shmaddr || shmaddr == (void *)-1){ return -1; }
	upper_context * uc = (upper_context *) shmaddr;
	for(;;){
		if(uc->client_pid != 0){
			printf("%s: waiting...\n", __FUNCTION__);
			sleep(1);
		}
		else{
			printf("%s: Enter text:\n", __FUNCTION__);
			fgets(uc->buffer, sizeof(uc->buffer)  - 1, stdin);
			uc->client_pid = 1;

			if(0 == strncmp(uc->buffer, "end", 3))
				break;
		}
	}
	int r = shmdt(shmaddr);
	if(r == -1){ return -1; }
	r = shmctl(shmid, IPC_RMID, 0);
	if(r == -1){ return -1; }
	return 0;
}

int test_pipe_1(int argc, char ** argv)
{
	char const * cmd = argc > 1? argv[1] : "ls";
	//捕获外部程序输出
//	char buffer[BUFSIZ] = "";
//	FILE * file = popen(cmd, "r");
//	if(file){
//		int n = fread(buffer, sizeof(char), BUFSIZ, file);
//		if(n > 0){
//			printf ("cmd = %s, read = \n%s\n", cmd , buffer);
//		}
//		pclose(file);
//
//	}
	//程序输出送到其它程序
	char buffer[BUFSIZ] = "/home/jun/ws/";
	FILE * file = popen(cmd, "w");
	if(file){
		int n = fwrite(buffer, sizeof(char), strlen(buffer), file);
		if(n > 0){
			printf ("cmd = %s, write = \n%s\n", cmd , buffer);
		}
		pclose(file);

	}
	return 0;
}
int test_pipe_2(int argc, char ** argv)
{
	int fdp[2];
	int r = pipe(fdp);
	if(r == 0){
		char buffer[BUFSIZ + 1] = "hello, pipe\n";
		int n = write(fdp[1], buffer, strlen(buffer));
		memset(buffer, '0', sizeof(buffer));
		if(n > 0){
			int m = read(fdp[0], buffer, n);
			buffer[m] = '\0';
		}
		printf("%s: buffer now = %s\n", __FUNCTION__, buffer);
	}
	return 0;
}

int test_pipe_3(int argc, char ** argv)
{
	int fdp[2];
	int r = pipe(fdp);
	if(r == 0){
		pid_t pid = fork();
		if(pid == -1){
			return -1;
		}
		if(pid == 0){
			printf("%s: child\n", __FUNCTION__);
			char buffer[BUFSIZ + 1] = "";
			int m = read(fdp[0], buffer, BUFSIZ);
			buffer[m] = '\0';
			printf("%s: buffer from parent = %s\n", __FUNCTION__, buffer);
		}
		else{
			printf("%s: parent\n", __FUNCTION__);
			char buffer[BUFSIZ + 1] = "hello, pipe\n";
			int n = write(fdp[1], buffer, strlen(buffer));
		}
	}	return 0;
}

int test_pipe_4(int argc, char ** argv)
{
	int fdp[2];
	int r = pipe(fdp);
	if(r == 0){
		pid_t pid = fork();
		if(pid == -1){
			return -1;
		}
		if(pid == 0){
			printf("%s: child\n", __FUNCTION__);
			char buffer[BUFSIZ + 1] = "hello, pipe\n";
			int n = write(fdp[1], buffer, strlen(buffer));
		}
		else{
			printf("%s: parent\n", __FUNCTION__);
			char buffer[BUFSIZ + 1] = "";
			int m = read(fdp[0], buffer, BUFSIZ);
			buffer[m] = '\0';
			printf("%s: buffer from child = %s\n", __FUNCTION__, buffer);
		}
	}	return 0;
	return 0;
}

int test_pipe_5(int argc, char ** argv)
{
	int fdp[2];
	int r = pipe(fdp);
	if(r == 0){
		pid_t pid = fork();
		if(pid == -1){
			return -1;
		}
		if(pid == 0){
			close(0);
			dup(fdp[0]);
			close(fdp[0]);
			close(fdp[1]);

			execlp("od", "od", "-c", 0);
		}
		else{
			close(fdp[0]);
			printf("%s: parent\n", __FUNCTION__);
			char buffer[BUFSIZ + 1] = "hello, pipe\n";
			int n = write(fdp[1], buffer, strlen(buffer));
			close(fdp[1]);
		}
	}	return 0;
}

int test_pipe_server(int argc, char ** argv)
{
	int fdp = -1;
	int r = access(pipe_file, F_OK);
	if(r == -1){
		r = mkfifo(pipe_file, 0777);
		if(r == -1)
			return -1;
	}
	printf("%s opening for write, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file, getpid());
	fdp = open(pipe_file, O_WRONLY);
	if(fdp == -1){
		return -1;
	}

	ssize_t sent = 0;
	char buffer[1024];
	printf("%s opened, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file, getpid());
	for(ssize_t n ; /*sent < PIPE_BUF*/n > 0; ){
		n = write(fdp, buffer, sizeof(buffer));
		if(n == -1){
			break;
		}
		sent += n;
	}

	printf("%s exited, pid = %u, written = %ld\n", __FUNCTION__, getpid(), sent);
	return 0;
}
int test_pipe_client(int argc, char ** argv)
{
	int r = access(pipe_file, F_OK);
	if(r == -1)
		return -1;
	printf("%s opening for read from server, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file, getpid());
	int fdp = open(pipe_file, O_RDONLY);
	printf("%s opened, FIFO = %s, pid = %u\n ", __FUNCTION__, pipe_file, getpid());
	if(fdp == -1)
		return -1;

	ssize_t total = 0;
	char buffer[PIPE_BUF * 2];
	for(ssize_t n = 1; n > 0;){
		n = read(fdp, buffer, sizeof(buffer));
		if(n == -1){
			break;
		}
		total += n;
	}
	printf("%s exited, pid = %u, read = %ld\n", __FUNCTION__, getpid(), total);
	return 0;
}

int test_pipe_server_2(int argc, char ** argv)
{
	int fdp = -1;
	int r = access(pipe_file_2, F_OK);
	if(r == -1){
		r = mkfifo(pipe_file_2, 0777);
		if(r == -1)
			return -1;
	}
	printf("%s opening for read from client, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_2, getpid());
	fdp = open(pipe_file_2, O_RDONLY);
	if(fdp == -1){
		return -1;
	}

	ssize_t total = 0;
	for(ssize_t n = 0;;){
		upper_context uc = {0};

		n = read(fdp, &uc, sizeof(upper_context));
		if(n == 0)
			continue;
		if(n == -1){
			printf("%s read from client failed, continue, pid = %u\n", __FUNCTION__, getpid());
			continue;
		}
		printf("%s read from client total = %ld, client-pid = %u\n", __FUNCTION__, n, uc.client_pid);

		for(int i = 0; i < sizeof(uc.buffer) / sizeof(uc.buffer[0]); ++i){
			uc.buffer[i] = toupper(uc.buffer[i]);
		}

		char const * pipe_file_client = fdp_make("fifo_%d", uc.client_pid);
		printf("%s opening write to client, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_2, getpid());
		int client_fdp = open(pipe_file_client, O_WRONLY);
		if(client_fdp == -1){
			printf("%s open fifo failed, continue, fifo = %s, pid = %u\n", __FUNCTION__, pipe_file_client, getpid());
			continue;
		}

		r = write(client_fdp, &uc, sizeof(upper_context));
		if(r == -1){
			printf("%s write to client failed, continue, pid = %u\n", __FUNCTION__, getpid());
			continue;
		}
		printf("%s written, upper_context = { .client_pid = %d, .buffer = %s}\n",
					__FUNCTION__, uc.client_pid, uc.buffer);
		close(client_fdp);
		total += n;
	}
	return 0;
}

inline const char* fdp_make(const char* format, ...) {
	static char ret[128] = "";
	va_list args1;
	va_start(args1, format);
	vsprintf(ret, format, args1);
	va_end(args1);
	return ret;
}

int test_pipe_client_2(int argc, char ** argv)
{
	upper_context uc = {getpid(), "hello, server"};

	char const * pipe_file_client = fdp_make("fifo_%d", uc.client_pid);
	int r = mkfifo(pipe_file_client, 0777);
	if(r == -1){
		printf("%s mkfifo failed, fifo = %s, pid = %u\n", __FUNCTION__, pipe_file_client, getpid());
		return -1;
	}

	int fdp = -1;
	r = access(pipe_file_2, F_OK);
	if(r == -1){
		printf("%s access failed!, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_2, getpid());
		return -1;
	}
	printf("%s opening for write to server, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_2, getpid());
	fdp = open(pipe_file_2, O_WRONLY);
	if(fdp == -1){
		printf("%s open failed!, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_2, getpid());
		return -1;
	}

	ssize_t n = write(fdp, &uc, sizeof(upper_context));
	if(n == -1){
		return -1;
	}
	printf("%s sent total = %ld, upper_context = { .client_pid = %d, .buffer = %s}\n",
			__FUNCTION__, n, uc.client_pid, uc.buffer);
	close(fdp);

	printf("%s opening for read response from server, FIFO = %s, pid = %u\n", __FUNCTION__, pipe_file_client, getpid());
	int client_fdp = open(pipe_file_client, O_RDONLY);
	if(client_fdp == -1)
		return -1;

	memset(&uc, '\0', sizeof(uc));
	n = read(client_fdp, &uc, sizeof(upper_context));
	if(n == -1){
		printf("%s read from server failed, exit, pid = %u\n", __FUNCTION__, getpid());
		return -1;
	}
	printf("%s exited, pid = %u, upper_context = { .client_pid = %d, .buffer = %s}\n",
			__FUNCTION__, getpid(), uc.client_pid, uc.buffer);

	close(client_fdp);
	remove(pipe_file_client);
	return 0;
}

int test_dup_dup2(int argc, char ** argv)
{
	char const * fname = __FUNCTION__;
	int oldfd;
	oldfd = open(fname, O_RDWR|O_CREAT,0644);
	dup2(oldfd,1);   //复制oldfd到文件描述符1（stdout标准输出）
	close(oldfd);    //关闭文件描述符oldfd
	printf("ddd");  //在标准输出上打印出ddd，这时由于标准输出已经被oldfd文件描述符代替
	return 0;       //打印到标准输出上的内容就全部打印到了文件mytest2中
}

