/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _URL_UTIL_H_
#define _URL_UTIL_H_
#include <stdio.h>

/* like printf except print \n as '\n', \0 as '\0' */
void fdump_str(FILE * f, char const * buf, size_t len, char const * beg, char const * end);
char * sdump_str(char * out, char const * buf, size_t len, char const * beg, char const * end);
/* format: <beg>chr<end> */
void fdump_chr(FILE * f, char chr, char const * beg, char const * end);
int sdump_chr(char * buf, char chr, char const * beg, char const * end);

/* usually for debug */
char const * dumpstr(char const * buf, size_t len, size_t dumplen);
#endif /*_URL_UTIL_H_*/
