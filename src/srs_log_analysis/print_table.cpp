/*!
 * This file is PART of srs_log_analysis
 * ouput results table
 */
#include "srs_log_analysis.h"	/*srs_trans, ...*/
#include <stdio.h>
#include <vector>		/*std::vector*/
#include <algorithm>	/*std::find_if*/
#include <time.h>		/*strftime*/
#include "net_util.h"	/*netutil_get_ip_str*/

void fprint_srs_log_stats(FILE * stream, std::vector<srs_trans> const& trans_stats,
		std::vector<srs_connect> const& cstats);

void fprint_srs_log_stats(FILE * stream, std::vector<srs_trans> const& trans_stats,
		std::vector<srs_connect> const& cstats)
{
	fprintf(stream, "%-21s%-16s%-14s%-14s%-14s%-14s\n", "time_stamp", "client_ip", "obytes", "ibytes", "okbps", "ikbps");
	for(auto & item : trans_stats){
		char buf1[32], buf2[32];
		strftime(buf1, sizeof(buf1), "%Y-%m-%d %H:%M:%S", localtime(&item.time_stamp));
		uint32_t ip = 0;
		auto find_by_sid = [&item](srs_connect const& c){ return item.sid == c.sid; };
		auto iter = std::find_if(cstats.begin(), cstats.end(), find_by_sid);
		if(iter != cstats.end())
			ip = iter->ip;
		fprintf(stream, "%-21s%-16s%-14zu%-14zu%-14zu%-14zu\n",
				buf1, netutil_get_ip_str(ip, buf2, sizeof(buf2))
				, item.obytes, item.ibytes
				, item.okbps, item.ikbps
				);
	}
}
