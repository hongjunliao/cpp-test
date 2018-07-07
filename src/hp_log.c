/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>, @date 2017/9/11
 *
 * log
 * */
#include <stdio.h>
#include <stdarg.h>      /* va_list, ... */
#include <sys/time.h>    /* gettimeofday */
#include <time.h>        /* time.h */
#include <unistd.h>      /* getpid, isatty */

/* log level */
extern int gloglevel;

void hp_log(void * f, char const * fmt, ...)
{
	FILE * fp = (FILE *)f;

	if(!(fp && fmt)) return;

	int color = (fileno(fp) == fileno(stderr)? 31 : 0); /* 31 for red, 0 for default */

	char buf[1024];

	struct timeval tv;
	gettimeofday(&tv, NULL);
	pid_t pid = getpid();

	int off1 = strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
	int off2 = snprintf(buf + off1, sizeof(buf) - off1, ".%03d]/%d ",
			(int) tv.tv_usec / 1000, pid);

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf + off1 + off2, sizeof(buf) - (off1 + off2), fmt, ap);
	va_end(ap);

	if(isatty(fileno(fp)))
		fprintf(fp, "\e[%dm%s\e[0m", color, buf);
	else fputs(buf, fp);

	fflush(fp);
}
