/*UNIX网络编程卷2:进程间通信,第2章 POSIX IPC*/
#include "../bd_test.h"
#include <stdio.h>
#include <mqueue.h>	/*POSIX消息队列*/
#include <semaphore.h> /*POSIX信号量*/
#include <sys/mman.h> /*POSIX共享区*/
#include <fcntl.h>
#include <unistd.h>	/*sysconf*/
int test_cygwin_posix_ipc_main(int argc, char ** arg)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
	mq_open("/tmp/mq1", O_RDONLY);
	fprintf(stdout, "MQ_OPEN_MAX = %d, MQ_PRIO_MAX = %d\n", sysconf(_SC_MQ_OPEN_MAX), sysconf(_SC_MQ_PRIO_MAX));

	int ret = 0;
	return ret;
}
