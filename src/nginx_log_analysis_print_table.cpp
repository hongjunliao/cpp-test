/*!
 * This file is PART of nginx_log_analysis
 * ouput results table
 */
#include <stdio.h>
#include <string.h> 	/*strncpy*/
#include <netinet/in.h>
#include <map>	/*std::map*/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "test_options.h"	/*nla_options**/
#include "string_util.h"	/*sha1sum*/
#include "net_util.h"	/*netutil_get_ip_str*/
#include "ipmap.h"		/*ipmap_lookup*/

/*all options: test_options.cpp*/
extern struct nla_options nla_opt;


/*flow table*/
static void print_flow_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot url*/
static void print_url_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot ip*/
static void print_ip_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*httpstatus_statistics*/
static void print_http_stats_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*ip_slowfast*/
static void print_ip_slowfast_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*cutip_slowfast*/
static void print_cutip_slowfast_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*url_key*/
static void print_url_key_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*ip_source*/
static void print_ip_source_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);

static void print_flow_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		log_stat const& stat = item.second;
		char line[512];
		/*format: site_id, datetime, device_id, num_total, bytes_total, user_id, pvs_m, px_m */
		/*FIXME: use fprintf?*/
		int sz = snprintf(line, 512, "%d %s %d %ld %zu %d %ld %zu\n",
				site_id, item.first.c_str("%Y%m%d%H%M"), device_id, stat.access_total()
				, stat.bytes_total(), user_id, stat._access_m, stat._bytes_m);
		if(sz <= 0){
			++i;
			continue;
		}
		buff += line;
		if(buff.size() > 1024 * 1024 * 16){
			size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
			if(r != buff.size())
				fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
						__FUNCTION__, buff.size(), r);
			buff.clear();
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_url_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		log_stat const& stat = item.second;
		for(auto const& url_item : stat._url_stats){
			auto const& url = url_item.first;
			auto const& st = url_item.second;
			size_t num_total = st.access_total(), num_200 = st.access(200), size_200 = st.bytes(200)
					, num_206 = st.access(206), size_206 = st.bytes(206), num_301302 = st.access(301, 302)
					, num_304 = st.access(304) , num_403 = st.access(403), num_404 = st.access(404), num_416 = st.access(416)
					, num_499 = st.access(499), num_500 = st.access(500), num_502 = st.access(502)
					, num_other = num_total - (num_200 + num_206 + num_301302 + num_304 + num_403
									+ num_404 + num_416 + num_499 + num_500 + num_502)
					;
			char line[512];
			/*format:
			 *datetime, url_key, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
			 *, num_403, num_404, num_416, num_499, num_500, num_502, num_other*/
			int sz = snprintf(line, 512, "%s %s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
						item.first.c_str("%Y%m%d%H%M"), sha1sum(url/*, strlen(url)*/)
						, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
						, num_403, num_404, num_416, num_499, num_500, num_502
						, num_other
						);
			if(sz <= 0){
				++i;
				continue;
			}
			buff += line;
			if(buff.size() > 1024 * 1024 * 16){
				size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
				if(r != buff.size())
//					fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n", __FUNCTION__, buff.size(), r);
				buff.clear();
			}
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
//		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n", __FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_ip_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	int i = 0;
	for(auto const& item : stats){
		for(auto const& ip_item : item.second._ip_stats){
			auto const& ipstat = ip_item.second;
			char ipbuff[20] = "0.0.0.0";
			/*format: site_id, device_id, ip, datetime, num*/
			/*FIXME: ip is string format?*/
			auto sz = fprintf(stream, "%d %d %s %s %zu\n",
							site_id, device_id, item.first.c_str("%Y%m%d%H%M"),
							netutil_get_ip_str(ip_item.first, ipbuff, sizeof(ipbuff)), ipstat.access);
			if(sz <= 0) ++i;
		}
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_http_stats_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		std::unordered_map<int, size_t> st;	/*http_status_code: access_count*/
		for(auto const& url_item : item.second._url_stats){
			for(auto const& httpcode_item : url_item.second._status){
				st[httpcode_item.first] += httpcode_item.second;
			}
		}
		char line[512];
		for(auto const& st_item : st){
			/*format: site_id, device_id, httpstatus, datetime, num*/
			int sz = snprintf(line, 512, "%d %d %d %s %zu\n",
					site_id, device_id, st_item.first, item.first.c_str("%Y%m%d%H%M"), st_item.second);
			if(sz <= 0){
				++i;
				continue;
			}
			buff += line;
			if(buff.size() > 1024 * 1024 * 16){
				size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
				if(r != buff.size())
					fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
							__FUNCTION__, buff.size(), r);
				buff.clear();
			}
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_ip_slowfast_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		char line[512];
		for(auto const& ip_item : item.second._ip_stats){
			auto const & ipstat = ip_item.second;
			double speed = (double)ipstat.bytes / ipstat.sec;
			/*format: device_id, ip, datetime, speed, type(miss,hit)*/
			/*FIXME: type?*/
			int sz = snprintf(line, 512, "%d %u %s %.0f %d\n",
					device_id, ip_item.first, item.first.c_str("%Y%m%d%H%M"), speed, 0);
			if(sz <= 0){
				++i;
				continue;
			}
			buff += line;
			if(buff.size() > 1024 * 1024 * 16){
				size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
				if(r != buff.size())
					fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
							__FUNCTION__, buff.size(), r);
				buff.clear();
			}
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_cutip_slowfast_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{

}

static void print_ip_source_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	static bool is_ipmap_loaded = false;
	static struct ipmap_ctx g_ipmap_ctx;
	if(!is_ipmap_loaded){
		if (0 != ipmap_load(nla_opt.ipmap_file, &g_ipmap_ctx, 1))  {
			fprintf(stderr, "%s: ipmap(%s) failed\n", __FUNCTION__, nla_opt.ipmap_file);
			return;
		}
		is_ipmap_loaded = true;
	}
	int i = 0;
	for(auto const& item : stats){
		for(auto const& ip_item : item.second._ip_stats){
			auto const & ipstat = ip_item.second;
			in_addr_t ip = ip_item.first;
			char sispbuff[32] = "- -";
			char ispbuff[32] = "- -";
		    auto isp = ipmap_nlookup(&g_ipmap_ctx, ip);
		    if (isp) {
		        auto str_sip = ipmap_tostr2(isp, ispbuff);
		        bool f = strcmp(str_sip, "CN") && strcmp(str_sip, "CA") && strcmp(str_sip, "US");
		        char const * fmt = f? "%s -" : "%s", *  param = f? str_sip : ispbuff;
				snprintf(sispbuff, 32, fmt, param);
		        fprintf(stdout, "%s: str_sip=%s, ispbuff=%s, sispbuff=%s\n", __FUNCTION__, str_sip, ispbuff, sispbuff);
		    }
		}
	}

}

int print_stats(std::map<time_interval, log_stat>const& logstats,
		int device_id, int site_id, int user_id)
{
	FILE * stream = stdout;
	char const * f = nla_opt.output_file_flow;
	if(nla_opt.flow){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_flow_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_flow '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_url_popular;
	if(nla_opt.url_popular){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_url_popular_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_url_popular '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_ip_popular;
	if(nla_opt.ip_popular){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_ip_popular_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_ip_popular '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_http_stats;
	if(nla_opt.http_stats){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_http_stats_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_http_stats '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_ip_slowfast;
	if(f){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_ip_slowfast_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_ip_slowfast '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_cutip_slowfast;
	if(f){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_cutip_slowfast_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_cutip_slowfast '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_ip_source;
	if(f){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_ip_source_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_ip_source '%s' for append failed\n", __FUNCTION__, f);
	}
	return 0;
	//	result = print_stats(stdout, logstats, -1);

}
