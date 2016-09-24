#include "termio_util.h"
#include <stdio.h>
#include <termios.h>	/*termios*/
#include <errno.h>	/*EINTR*/
#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)

int set_disp_mode(int fd, int option)
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

int getpasswd(char const * msg, char* passwd, int size)
{
   int c;
   int n = 0;

   fprintf(stdout, "%s:", msg? msg : "Please Input password:");
   fflush(stdout);
   do{
      c=getchar();
      if (c != '\n'|c!='\r'){
         passwd[n++] = c;
      }
   }while(c != '\n' && c !='\r' && n < (size - 1));
   passwd[n] = '\0';
   return n;
}
