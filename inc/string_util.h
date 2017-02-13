/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_
#include <stdio.h>

#if (defined __GNUC__) && (!defined __CYGWIN__)
/*to lower string*/
char *strlwr(char *s);
/*to upper string*/
char *strupr(char *s);
#endif	/*(defined __GNUC__) && !(defined __CYGWIN__)*/

/*string_util.cpp*/
/*_r version is thread-safe*/

/* checksum.cpp */
#if (defined __GNUC__ || defined  __CYGWIN_GCC__)
/*@param buff[33] at least(include NULL)*/
char * md5sum_r(char const * str, int len, char * buff);
/*@param buff[41] at least(include NULL)*/
char * sha1sum_r(char const * str, int len, char * buff);
#endif /* (defined __GNUC__ || defined  __CYGWIN_GCC__) */


char const * byte_to_mb_kb_str(size_t bytes, char const * fmt);
char * byte_to_mb_kb_str_r(size_t bytes, char const * fmt, char * buff);

#endif /*_STRING_UTIL_H_*/
