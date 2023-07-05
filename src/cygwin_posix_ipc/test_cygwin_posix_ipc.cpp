#ifdef CPP_TEST_WITH_CYGWIN_POSIX_IPC

/*UNPv2,POSIX IPC*/
#if (defined __GCC__ || defined  __CYGWIN_GCC__)

#include "bd_test.h"
#include <stdio.h>
#include <mqueue.h>	/*POSIX*/
#include <semaphore.h> /*POSIX*/
#include <sys/mman.h> /*POSIX mmap*/
#include <fcntl.h>
#include <unistd.h>	/*sysconf*/
int test_cygwin_posix_ipc_main(int argc, char ** arg)
{
	fprintf(stdout, "%s\n", __FUNCTION__);
	mq_open("/tmp/mq1", O_RDONLY);	/*gcc link option: -lrt*/
	fprintf(stdout, "MQ_OPEN_MAX = %d, MQ_PRIO_MAX = %d\n", sysconf(_SC_MQ_OPEN_MAX), sysconf(_SC_MQ_PRIO_MAX));

	int ret = 0;
	return ret;
}
#endif

#endif

