/*for get local ip*/
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

static int test_get_if_addrs_main(int argc, char ** argv)
{
	char ips[64][16];
	int count = 64;
	int result  = get_if_addrs(ips[0], count, 16);
	if(result != 0)
		return -1;
	for(int i = 0; i < count; ++i){
		fprintf(stdout, "%s\n", ips[i]);
	}
    return 0;
}
