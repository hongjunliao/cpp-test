/*!
 * This file is PART of nginx_log_analysis
 * ouput results table
 */
#include <stdio.h>
#include <limits.h>	/*PATH_MAX*/
#include <string.h> 	/*strncpy*/
#include <netinet/in.h>
#include <map>			/*std::map*/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "test_options.h"	/*nla_options**/
#include "string_util.h"	/*sha1sum*/
#include "net_util.h"	/*netutil_get_ip_str*/

/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

/*flow table*/
static void print_flow_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot url*/
static void print_url_popular_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
static void print_url_popular_table(char const * folder, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot ip*/
static void print_ip_popular_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*httpstatus_statistics*/
static void print_http_stats_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*ip_slowfast*/
static void print_ip_slowfast_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*cutip_slowfast*/
static void print_cutip_slowfast_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*url_key*/
static void print_url_key_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*ip_source*/
static void print_ip_source_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);
static void print_ip_source_table(char const * folder, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id);

static void print_flow_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	size_t i = 0;
	for(auto const& item : stats){
		auto const& stat = item.second;
		char line[512];
		/*format: site_id, datetime, device_id, num_total, bytes_total, user_id, pvs_m, px_m */
		/*FIXME: use fprintf?*/
		auto sz = snprintf(line, 512, "%d %s %d %ld %zu %d %ld %zu\n",
				site_id, item.first.c_str("%Y%m%d%H%M"), device_id, stat.access_total()
				, stat.bytes_total(), user_id, stat._access_m, stat._bytes_m);
		if(sz <= 0){
			++i;
			continue;
		}
		buff += line;
		if(buff.size() > 1024 * 1024 * 16){
			auto r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
			if(r != buff.size())
				fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
						__FUNCTION__, buff.size(), r);
			buff.clear();
		}
	}
	auto r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

void do_print_url_popular_table(FILE * stream, time_group const& g, log_stat const& stat, size_t& i)
{
	for(auto const& url_item : stat._url_stats){
		auto const& url = url_item.first;
		auto const& st = url_item.second;
		auto num_total = st.access_total(), num_200 = st.access(200), size_200 = st.bytes(200)
				, num_206 = st.access(206), size_206 = st.bytes(206), num_301302 = st.access(301, 302)
				, num_304 = st.access(304) , num_403 = st.access(403), num_404 = st.access(404), num_416 = st.access(416)
				, num_499 = st.access(499), num_500 = st.access(500), num_502 = st.access(502)
				, num_other = num_total - (num_200 + num_206 + num_301302 + num_304 + num_403
								+ num_404 + num_416 + num_499 + num_500 + num_502)
				;
		/*format:
		 *datetime, url_key, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
		 *, num_403, num_404, num_416, num_499, num_500, num_502, num_other*/
//			char sha1buf[33];
		auto sz = fprintf(stream, "%s %s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
					g.c_str("%Y%m%d%H%M"), url.c_str()/*sha1sum_r(url.c_str(), url.size(), sha1buf)*/
					, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
					, num_403, num_404, num_416, num_499, num_500, num_502
					, num_other
					);
		if(sz <= 0) ++i;
	}
}

inline void print_url_popular_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;
	for(auto const& item : stats){
		auto const& stat = item.second;
		do_print_url_popular_table(stream, item.first, stat, i);
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

static void print_url_popular_table(char const * folder, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;

	for(auto const& item : stats){
		auto const& stat = item.second;
		std::unordered_map<std::string, std::string> argmap = {
				{"datetime",  item.first.c_str("%Y%m%d%H%M")},
				{"site_id", std::to_string(site_id)},
				{"device_id", std::to_string(device_id)},
				{"user_id", std::to_string(user_id)},
		};
		std::string fname = folder, outname;
		parse_fmt(nla_opt.output_file_url_popular_format, outname, argmap);
		fname += '/'; fname += outname;

		auto stream  = fopen(fname.c_str(), "w");
		if(!stream) continue;
		do_print_url_popular_table(stream, item.first, stat, i);
		fclose(stream);
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);

}

inline void print_ip_popular_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;
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
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

inline void print_http_stats_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	int i = 0;
	for(auto const& item : stats){
		std::unordered_map<int, size_t> st;	/*http_status_code: access_count*/
		for(auto const& url_item : item.second._url_stats){
			for(auto const& httpcode_item : url_item.second._status){
				st[httpcode_item.first] += httpcode_item.second;
			}
		}
		for(auto const& st_item : st){
			/*format: site_id, device_id, httpstatus, datetime, num*/
			auto sz = fprintf(stream, "%d %d %d %s %zu\n",
					site_id, device_id, st_item.first, item.first.c_str("%Y%m%d%H%M"), st_item.second);
			if(sz <= 0) ++i;
		}
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_ip_slowfast_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	/*FIXME: print only top 10?*/
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		char line[512];
		for(auto const& ip_item : item.second._ip_stats){
			auto const & ipstat = ip_item.second;
			double speed = (double)ipstat.bytes / ipstat.sec;
			/*format: device_id, ip, datetime, speed, type(MISS,HIT)*/
			/*FIXME: type?*/
			auto sz = snprintf(line, 512, "%d %u %s %.0f %d\n",
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
	auto r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_cutip_slowfast_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;
	for(auto const& item : stats){
		for(auto const& cutip_item : item.second._cuitip_stats){
			auto const & cutipstat = cutip_item.second;
			double speed = cutipstat.sec > 0? (double)cutipstat.bytes / cutipstat.sec : cutipstat.bytes * 1000000.0;
			/*format: device_id, datetime, ip, speed*/
			auto sz = fprintf(stream, "%d %s %s %.0f\n",
					device_id, item.first.c_str("%Y%m%d%H%M"), cutip_item.first.c_str(), speed);
			if(sz <= 0) ++i;
		}
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

static void print_url_key_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	//NOT implement yet
}


void do_print_ip_source_table(FILE * stream, time_group const& g, log_stat const& stat, size_t& i, int device_id)
{
	for(auto const& li_item : stat._locisp_stats){
		auto const & li = li_item.first;
		auto const & listat = li_item.second;
		char loc_isp[7 + 1 + 3 + 1 + 20]; /*<loc><blank><isp><NULL><?>*/
		/*format: bw_time, local_id, isp_id, pvs, tx, pvs_m, tx_m, device_id*/
		auto sz = fprintf(stream, "%s %s %zu %zu %zu %zu %d\n",
						g.c_str("%Y%m%d%H%M"),
						li.loc_isp_c_str(loc_isp, sizeof(loc_isp)),
						listat.access, listat.bytes,
						listat.access_m, listat.bytes_m, device_id);
		if(sz <= 0) ++i;
	}
}

static void print_ip_source_table(char const * folder, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;
	for(auto const& item : stats){
		auto const& stat = item.second;
		std::unordered_map<std::string, std::string> argmap = {
				{"datetime",  item.first.c_str("%Y%m%d%H%M")},
				{"site_id", std::to_string(site_id)},
				{"device_id", std::to_string(device_id)},
				{"user_id", std::to_string(user_id)},
		};
		std::string fname = folder, outname;
		parse_fmt(nla_opt.output_file_ip_source_format, outname, argmap);
		fname += '/'; fname += outname;

		auto stream  = fopen(fname.c_str(), "w");
		if(!stream) continue;
		do_print_ip_source_table(stream, item.first, stat, i, device_id);
		fclose(stream);
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

static void print_ip_source_table(FILE * stream, std::map<time_group, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	size_t i = 0;
	for(auto const& item : stats){
		do_print_ip_source_table(stream, item.first, item.second, i, device_id);
	}
	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, i);
}

int print_stats(std::map<time_group, log_stat>const& logstats,
		int device_id, int site_id, int user_id)
{
	auto stream = stdout;
	auto f = nla_opt.output_file_flow;
	if(f){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_flow_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_flow '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_url_popular;
	if(f){
		bool is_to_stdout = strcmp(f, "1") == 0,
			is_split = !is_to_stdout && nla_opt.output_file_url_popular_split;
		if(is_split)
			print_url_popular_table(f, logstats, device_id, site_id, user_id);
		else {
			if(is_to_stdout || (stream = fopen(f, "a")) != NULL)
				print_url_popular_table(stream, logstats, device_id, site_id, user_id);
			else
				fprintf(stderr, "%s: fopen output_file_url_popular '%s' for append failed\n", __FUNCTION__, f);
		}
	}
	f = nla_opt.output_file_ip_popular;
	if(f){
		if(strcmp(f, "1") == 0 || (stream = fopen(f, "a")) != NULL)
			print_ip_popular_table(stream, logstats, device_id, site_id, user_id);
		else
			fprintf(stderr, "%s: fopen output_file_ip_popular '%s' for append failed\n", __FUNCTION__, f);
	}
	f = nla_opt.output_file_http_stats;
	if(f){
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
		bool is_to_stdout = strcmp(f, "1") == 0,
			is_split = !is_to_stdout && nla_opt.output_file_ip_source_split;
		if(is_split)
			print_ip_source_table(f, logstats, device_id, site_id, user_id);
		else {
			if(is_to_stdout || (stream = fopen(f, "a")) != NULL)
				print_ip_source_table(stream, logstats, device_id, site_id, user_id);
			else
				fprintf(stderr, "%s: fopen output_file_ip_source '%s' for append failed\n", __FUNCTION__, f);
		}
	}
	return 0;
}
