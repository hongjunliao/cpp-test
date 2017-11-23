/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////
/* a string [beg, end) */
typedef struct {
	char * beg;
	char * end;
}str_t;

#define str_t_is_null(str) (!s.beg && !s.end)
#define str_t_printable(str) (std::string(str.beg, str.end).c_str())

int str_t_fprint(str_t const * s, FILE * f);

//////////////////////////////////////////////////////////////////////////////////

#if !(defined __GNUC__)
/*to lower string*/
char *strlwr(char *s);
/*to upper string*/
char *strupr(char *s);
#endif	/* defined __CYGWIN__ || (!defined _WIN32 && defined __GNUC__) */

/*string_util.cpp*/
/*_r version is thread-safe*/

/* checksum.cpp */
/*@param buff[33] at least(include NULL)*/
char * md5sum_r(char const * str, int len, char * buff);
/*@param buff[41] at least(include NULL)*/
char * sha1sum_r(char const * str, int len, char * buff);

/* @param f, file to calculate md5 */
char * md5sum_file_r(char const * f, char * buff);

//////////////////////////////////////////////////////////////////////////////////
char const * byte_to_mb_kb_str(size_t bytes, char const * fmt);
char * byte_to_mb_kb_str_r(size_t bytes, char const * fmt, char * buff);

/* like strrchr except that @param buf endwith '\0' NOT requried */
char const * strnrchr(char const * buf, int sz, char ch);

/* just like std::atoi */
int myatoi(char const * str, size_t len);

/* sort using qsort and strcmp */
void strutil_qsort(char ** strs, int size);
/*
 * find the maximum vlaue less than @param str
 * */
char const * strutil_findmax(char const * str, char ** strs, int size);
#endif /*_STRING_UTIL_H_*/
