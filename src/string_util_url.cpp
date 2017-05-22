/*!
 * This file is Part of cpp-test
 * @author: hongjun.liao<docici@126.com>
 */

#include "string_util.h"        /* str_t */
#include <boost/regex.hpp> 		/*  boost::regex_search */

int parse_domain_from_url(const char* url, char* domain)
{
	/*sample: 'rtmp://127.0.0.1:1359/'*/
	boost::cmatch cm;
	auto f = boost::regex_search(url, cm, boost::regex("://([^/:]+)(?::[0-9]+)?/"));
	if(!f) return -1;

	auto length = cm.length(1);
	strncpy(domain, cm[1].first, length);
	domain[length] = '\0';
	return 0;
}

int parse_domain_from_url(char const * url, str_t * domain)
{
//	fprintf(stderr, "%s: url='%s'\n", __FUNCTION__, url);
	auto c = strchr(url, ':');
	if(c && *(c + 1) == '/' && *(c + 2) == '/'){
		domain->beg = const_cast<char *>(c + 3);
		domain->end = strchr(domain->beg, '/');

//		str_t_fprint(domain, stdout);
		return 0;
	}
	return -1;
}
