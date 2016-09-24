/*UNIX�����̾�2:���̼�ͨ��,��2�� POSIX IPC*/
#include "bd_test.h"
#include <stdio.h>
#include <mqueue.h>	/*POSIX��Ϣ����*/
#include <semaphore.h> /*POSIX�ź���*/
#include <sys/mman.h> /*POSIX������*/
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
