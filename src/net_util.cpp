/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 * for get local ip
 */
#if(!defined _WIN32 && (defined __GNUC__ || defined  __CYGWIN_GCC__))
#include "net_util.h"	/**/
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>	/*struct ifaddrs*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
/*implementation from shell: man getifaddrs, @param count rows, @param sz cows*/
int get_if_addrs(char *ips, int & count, int sz)
{
    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        return -1;
    }
    int i = 0;
    for (ifaddrs * ifa = ifaddr; ifa ; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;
        int family = ifa->ifa_addr->sa_family;
        if (!(family == AF_INET || family == AF_INET6)) {
        	continue;
        }
		char host[NI_MAXHOST];
        int s = getnameinfo(ifa->ifa_addr,
                (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                      sizeof(struct sockaddr_in6),
                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (s != 0) {
//          printf("getnameinfo() failed: %s\n", gai_strerror(s));
            continue;
        }
        strncpy((ips + i * sz), host, sz);
        ++i;
        if(i == count)
        	break;
    }
    count = i;
    freeifaddrs(ifaddr);
    return 0;
}

uint32_t netutil_get_ip_from_str(char const * ipstr)
{
	if(!ipstr)
		return 0;
	struct in_addr inaddr;
	if(inet_pton(AF_INET, ipstr, (void *)&inaddr) != 1)
		return 0;
	/*FIXME: byte order!!!*/
	return inaddr.s_addr;
}

char const * netutil_get_ip_str(uint32_t ip, char * buff, size_t len)
{
	struct in_addr inaddr;
	inaddr.s_addr = ip;
	return inet_ntop(AF_INET, (void *)&inaddr, buff, len);
}

int test_net_util_main(int argc, char ** argv)
{
	char ips[64][64];
	int count = 64;
	int result  = get_if_addrs(ips[0], count, 64);
	if(result != 0)
		return -1;
	for(int i = 0; i < count; ++i){
		fprintf(stdout, "%s: i=%d, ip=%s\n", __FUNCTION__, i, ips[i]);
	}
	char const * sip = "182.247.200.210";
	uint32_t ip = netutil_get_ip_from_str(sip);
	char ipbuff[16];
	fprintf(stdout, "%s: sip=%s, netutil_get_ip_from_str=%u, netutil_get_ip_str=%s\n", __FUNCTION__,
			sip, ip, netutil_get_ip_str(ip, ipbuff, 16));
    exit(0);
}
#endif /* (!defined _WIN32 && (defined __GNUC__ || defined  __CYGWIN_GCC__)) */
