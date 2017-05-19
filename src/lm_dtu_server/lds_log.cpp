/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * log system
 */
#include "lds_inc.h"  /*  */
#include <stdio.h>
#include <stdarg.h>   /* va_list */
#include <time.h>     /* ctime_s */

static char const * log_level[LDS_LOG_LEVEL_MAX + 1] = { "info", "debug", "error", "unknown"};
static char const * log_type[LDS_LOG_TYPE_MAX + 1] = { "app", "memory", "wqp", "unknown"};

int lds_log_to_file(int type, int level, char const * fmt, ...)
{
	time_t t = time(NULL);
	char str[64];
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", localtime(&t));
	/* log header */
	char buf[1024];
	int lt = type >= 0 && type < LDS_LOG_TYPE_MAX? type : LDS_LOG_TYPE_MAX;
	int ll = level >= 0 && level < LDS_LOG_LEVEL_MAX? level : LDS_LOG_LEVEL_MAX;

	int n = sprintf(buf, "[%s] %s/%s: ", str, log_type[lt], log_level[ll]);

	va_list args;
	va_start(args, fmt);
	int r = vsprintf(buf + n, fmt, args);
	va_end(args);

	fprintf((level <= LDS_LOG_DEBUG? stdout : stderr), "%s", buf);
	return r;
}

