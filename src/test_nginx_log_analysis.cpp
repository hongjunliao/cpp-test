#include "bd_test.h"
#include <time.h>
#include <stdio.h>
class time_mark{

};

int test_nginx_log_analysis_main(int argc, char ** argv)
{
	fprintf(stdout, "%s:\n", __FUNCTION__);
	struct tm my_tm;
	char const * stime = "17/Sep/2016:01:19:43 hello";
	char const * result = strptime(stime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	mktime(&my_tm);
	fprintf(stdout, "time=%s, NOT paused=%s, asctime:sec=%d,all=%s\n",
			stime, result, my_tm.tm_sec, asctime(&my_tm));
	return 0;
}
