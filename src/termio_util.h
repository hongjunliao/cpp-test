/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _TERMIO_UTIL_H_
#define _TERMIO_UTIL_H_
#include <stdio.h>
#ifdef __cplusplus
extern "C"{
#endif

/*!
 * set termios display
 * @fd: STDIN_FILENO
 * @option 0 close
 */
int termio_set_disp_mode(int fd, int option);
int termio_getpasswd(char const * msg, char* passwd, int size);
void termio_debug_log(FILE * stream, const char *fmt, ...);
int termio_fprintf(FILE * stream, char const * fmt, ...);
int termio_get_col_row(int * col, int * row);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_TERMIO_UTIL_H_*/
