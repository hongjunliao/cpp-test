/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_
#ifdef __cplusplus
extern "C"{
#endif

#if (defined __GNUC__) && !(defined __CYGWIN__)
/*to lower string*/
char *strlwr(char *s);
/*to upper string*/
char *strupr(char *s);
#endif	/*(defined __GNUC__) && !(defined __CYGWIN__)*/

#include <stdio.h>
/*string_util.cpp*/
extern char const * md5sum(char const * str, int len);
extern char const * sha1sum(char const * str, int len);
extern char const * byte_to_mb_kb_str(size_t bytes, char const * fmt);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_STRING_UTIL_H_*/
