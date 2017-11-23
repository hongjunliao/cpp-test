/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _URL_UTIL_H_
#define _URL_UTIL_H_
#include "string_util.h"
/* get domain from url,
 * sample get '127.0.0.1' from 'rtmp://127.0.0.1:1359/'
 * return 0 on success
 * @notes: @param domain big enough
 */
int parse_domain_from_url(char const * url, char * domain);
int parse_domain_from_url(char const * url, str_t * domain);
#endif /*_URL_UTIL_H_*/
