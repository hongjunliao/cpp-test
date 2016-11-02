/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */
#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_
#include <stdint.h> /*uint32_t*/
#include <stdio.h>	/*size_t*/
#ifdef __cplusplus
extern "C"{
#endif

/*net_util.cpp*/
extern int get_if_addrs(char *ips, int & count, int sz);
extern uint32_t netutil_get_ip_from_str(char const * ipstr);
extern char const * netutil_get_ip_str(uint32_t ip, char * buff, size_t len);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_NET_UTIL_H_*/
