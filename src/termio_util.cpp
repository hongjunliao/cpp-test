/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#if (defined __GCC__ || defined  __CYGWIN__)
#include "termio_util.h"
#include <stdio.h>
#include <time.h>	/*tmespec*/
#include <termios.h>	/*termios*/
#include <errno.h>	/*EINTR*/
#include <sys/ioctl.h> /*ioctl*/
#include <unistd.h> /*STDIN_FILENO*/
#include <stdarg.h> /*va_start*/


#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)

int termio_set_disp_mode(int fd, int option)
{
   int err;
   struct termios term;
   if(tcgetattr(fd,&term)==-1){
     fprintf(stderr, "%s: Cannot get the attribution of the terminal", __FUNCTION__);
     return 1;
   }
   if(option)
        term.c_lflag|=ECHOFLAGS;
   else
        term.c_lflag &=~ECHOFLAGS;
   err=tcsetattr(fd,TCSAFLUSH,&term);
   if(err==-1 && err==EINTR){
	   fprintf(stderr, "%s: Cannot set the attribution of the terminal", __FUNCTION__);
       return 1;
   }
   return 0;
}

int termio_getpasswd(char const * msg, char* passwd, int size)
{
   int c;
   int n = 0;

   fprintf(stdout, "%s:", msg? msg : "Please Input password:");
   fflush(stdout);
   do{
      c=getchar();
      if (c != '\n'|| c!='\r'){
         passwd[n++] = c;
      }
   }while(c != '\n' && c !='\r' && n < (size - 1));
   passwd[n] = '\0';
   return n;
}

int termio_get_col_row(int * col, int * row)
{
	struct winsize wsz;
	int result = ioctl(STDIN_FILENO, TIOCGWINSZ, &wsz);
	if(result == 0){
		if(col) *col = wsz.ws_col;
		if(row) *row = wsz.ws_row;
	}
	return result;
}

int termio_fprintf(FILE * stream, char const * fmt, ...)
{
    va_list args1;
    va_start(args1, fmt);
	int result = vfprintf(stream, fmt, args1);
	va_end(args1);
	return result;
}


void termio_debug_log(FILE * stream, const char *fmt, ...)
{
#ifdef __CYGWIN_GCC__
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    gethrestime(&ts);
    char time_buf[100];
    size_t rc = strftime(time_buf, sizeof time_buf, "%D %T", gmtime(&ts.tv_sec));
    snprintf(time_buf + rc, sizeof time_buf - rc, ".%06ld UTC", ts.tv_nsec / 1000);

    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    char buf[1+vsnprintf(NULL, 0, fmt, args1)];
    va_end(args1);
    vsnprintf(buf, sizeof buf, fmt, args2);
    va_end(args2);

    fprintf(stream, "%s [debug]: %s\n", time_buf, buf);
#endif /*__CYGWIN_GCC__*/
}

#ifndef NDEBUG
#include <unistd.h> /* isatty */
#include <stdio.h>
static int test_termio_main(int argc, char ** argv)
{
	bool istty = isatty(fileno(stdout));
	return 0;
}
#endif
#endif /* (defined __GCC__ || defined  __CYGWIN_GCC__) */
