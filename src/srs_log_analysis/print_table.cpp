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

void fprint_srs_log_stats(FILE * stream, std::unordered_map<std::string, srs_domain_stat> const& srs_stats)
{
	fprintf(stream, "%-8s%-13s%-16s%-40s%-14s%-14s\n", "site_id", "time", "client_ip", "url", "obytes", "ibytes");
	for(auto const& ds : srs_stats){
		for(auto const& stat : ds.second._stats){
			auto & s = stat.second;

			for(auto & url : s.urls){
				/*TODO*/
				char buf[32], buft[32];
//				stat.first.c_str_r(buft, sizeof(buft));
//				netutil_get_ip_str(s.ips[url.first], buf, sizeof(buf));
//				fprintf(stream, "%-8d%-13s%-16s%-40s%-14zu%-14zu\n",
//						ds.second._site_id
//						, buft
//						, buf
//						, url.second.c_str(), s.obytes[url.first], s.ibytes[url.first]
//						);

			}

		}
	}
}
