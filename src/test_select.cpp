/*
 * test_select.c
 *
 *  Created on: Mar 24, 2016
 *      Author: liaohj
 */
#ifdef __CYGWIN_GCC__
#include "bd_test.h"	//required!
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
int test_select_main(int argc, char ** argv)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fileno(stdin), &fds);
	struct timeval timeout = {
			.tv_sec = 8,
			.tv_usec = 0
	};
	char buf[1024] = "";
	int ret = select(2, &fds, (fd_set *)0, (fd_set *)0, &timeout);
	switch(ret){
	case 0:
		printf("%s select timedout\n", __FUNCTION__);
		break;
	case -1:
		printf("%s select failed\n", __FUNCTION__);
		break;
	default:
		if(FD_ISSET(fileno(stdin), &fds)){
			fgets(buf, 1023, stdin);
			printf("%s", buf);
		}
		break;
	}
	return 0;
}
#endif /*__CYGWIN_GCC__*/
