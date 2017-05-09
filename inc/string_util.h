/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////////
/* a string [beg, end) */
struct str_t {
	char * beg;
	char * end;
};

#define str_t_is_null(str) (!s.beg && !s.end)
#define str_t_printable(str) (std::string(str.beg, str.end).c_str())

int str_t_fprint(str_t const * s, FILE * f);
/* get domain from url,
 * sample get '127.0.0.1' from 'rtmp://127.0.0.1:1359/'
 * return 0 on success
 * @notes: @param domain big enough
 */
int parse_domain_from_url(char const * url, char * domain);
int parse_domain_from_url(char const * url, str_t * domain);

#if (defined __CYGWIN__ || (!defined _WIN32 && defined __GNUC__))
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


char const * byte_to_mb_kb_str(size_t bytes, char const * fmt);
char * byte_to_mb_kb_str_r(size_t bytes, char const * fmt, char * buff);

/* like strrchr except that @param buf endwith '\0' NOT requried */
char const * strnrchr(char const * buf, int sz, char ch);
/* like printf except print \n as '\n', \0 as '\0' */
void str_dump(FILE * f, char const * buf, size_t len);

#endif /*_STRING_UTIL_H_*/
