/*!
 * This file is PART of nginx_log_analysis
 * ouput results table
 * @author hongjun.liao <docici@126.com>
 * @date 2016/9
 */
#include <stdio.h>
#include <limits.h>		/*PATH_MAX*/
#include <string.h> 	/*strncpy*/
#include <netinet/in.h>
#include <map>			/*std::map*/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "test_options.h"		/*plcdn_la_options**/
#include "string_util.h"		/*sha1sum*/
#include "net_util.h"	/*netutil_get_ip_str*/

/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

/*nginx_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
/*main.cpp*/
extern time_t g_plcdn_la_start_time;
extern int g_plcdn_la_device_id;

static std::string parse_nginx_output_filename(char const * fmt, char const *interval, int site_id, int user_id);
/*flow table*/
static void print_flow_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*hot url*/
static void print_url_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*hot ip*/
static void print_ip_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*httpstatus_statistics*/
static void print_http_stats_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*ip_slowfast*/
static void print_ip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*cutip_slowfast*/
static void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*url_key*/
static void print_url_key_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*ip_source*/
static void print_ip_source_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);

static std::string parse_nginx_output_filename(char const * fmt, char const *interval, int site_id, int user_id)
{
	std::unordered_map<std::string, std::string> argmap;
	char buff[32] = "";
	strftime(buff, sizeof(buff), "%Y%m%d%H%M", localtime(&g_plcdn_la_start_time));

	argmap["device_id"] = std::to_string(g_plcdn_la_device_id);
	argmap["datetime"] = buff;

	argmap["interval"] = interval;
	argmap["site_id"] = std::to_string(site_id);
	argmap["user_id"] = std::to_string(user_id);

	std::string outname;
	parse_fmt(fmt, outname, argmap);
	return outname;
}

static void print_flow_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	/*format: site_id, datetime, device_id, num_total, bytes_total, user_id, pvs_m, px_m */
	auto sz = fprintf(stream, "%d %s %d %ld %zu %d %ld %zu\n",
			site_id, g.c_str("%Y%m%d%H%M"), g_plcdn_la_device_id, stat.access_total()
			, stat.bytes_total(), user_id, stat._access_m, stat._bytes_m);
	if(sz <= 0) ++n;
}

void print_url_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
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
		if(sz <= 0) ++n;
	}
}

inline void print_ip_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	for(auto const& ip_item : stat._ip_stats){
		auto const& ipstat = ip_item.second;
		if(plcdn_la_opt.min_ip_popular > 0 && ipstat.access < plcdn_la_opt.min_ip_popular)
			continue;
		if(plcdn_la_opt.max_ip_popular >= 0 && ipstat.access > plcdn_la_opt.max_ip_popular)
			continue;

		char ipbuff[20] = "0.0.0.0";
		/*format: site_id, device_id, ip, datetime, num*/
		/*FIXME: ip is string format?*/
		auto sz = fprintf(stream, "%d %d %s %s %zu\n",
						site_id, g_plcdn_la_device_id, g.c_str("%Y%m%d%H%M"),
						netutil_get_ip_str(ip_item.first, ipbuff, sizeof(ipbuff)), ipstat.access);
		if(sz <= 0) ++n;
	}
}

inline void print_http_stats_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	std::unordered_map<int, size_t> st;	/*http_status_code: access_count*/
	for(auto const& url_item : stat._url_stats){
		for(auto const& httpcode_item : url_item.second._status){
			st[httpcode_item.first] += httpcode_item.second;
		}
	}
	for(auto const& st_item : st){
		/*format: site_id, device_id, httpstatus, datetime, num*/
		auto sz = fprintf(stream, "%d %d %d %s %zu\n",
				site_id, g_plcdn_la_device_id, st_item.first, g.c_str("%Y%m%d%H%M"), st_item.second);
		if(sz <= 0) ++n;
	}
}

inline void print_ip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	return;
	/*FIXME: print only top 10?*/
	for(auto const& ip_item : stat._ip_stats){
		auto const & ipstat = ip_item.second;
		double speed = (ipstat.sec != 0? (double)ipstat.bytes / ipstat.sec : (double)ipstat.bytes * 1000000.0);
		/*format: device_id, ip, datetime, speed, type(MISS,HIT)*/
		/*FIXME: type?*/
		auto sz = fprintf(stream, "%d %u %s %.0f %d\n",
				g_plcdn_la_device_id, ip_item.first, g.c_str("%Y%m%d%H%M"), speed, 0);
		if(sz <= 0) ++n;
	}
}

inline void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	for(auto const& cutip_item : stat._cuitip_stats){
		auto const & cutipstat = cutip_item.second;
		double speed = cutipstat.sec > 0? (double)cutipstat.bytes / cutipstat.sec : cutipstat.bytes * 1000000.0;
		/*format: device_id, datetime, ip, speed*/
		auto sz = fprintf(stream, "%d %s %s %.0f\n",
				g_plcdn_la_device_id, g.c_str("%Y%m%d%H%M"), cutip_item.first.c_str(), speed);
		if(sz <= 0) ++n;
	}
}

static void print_url_key_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	//NOT implement yet
}

void print_ip_source_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
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
						listat.access_m, listat.bytes_m, g_plcdn_la_device_id);
		if(sz <= 0) ++n;
	}
}

static FILE * & append_stream(std::map<std::string, FILE *> & filemap, std::string const& filename)
{
	if(!filemap[filename])
		filemap[filename] = fopen(filename.c_str(), "a");	/*append mode*/
	return filemap[filename];
}

int print_plcdn_log_stats(std::unordered_map<std::string, nginx_domain_stat> const& stats)
{
	std::map<std::string, FILE *> filemap; /*for output filenames*/
	size_t n = 0;
	for(auto const& dstat : stats){
		for(auto const& item : dstat.second._stats){
			auto site_id = dstat.second._site_id, user_id = dstat.second._user_id;
			if(plcdn_la_opt.output_file_flow){
				auto outname = std::string(plcdn_la_opt.output_file_flow) +
						parse_nginx_output_filename(plcdn_la_opt.format_flow, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_flow_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_url_popular){
				auto outname = std::string(plcdn_la_opt.output_file_url_popular) +
						parse_nginx_output_filename(plcdn_la_opt.format_url_popular, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_url_popular_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_ip_popular){
				auto outname = std::string(plcdn_la_opt.output_file_ip_popular) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_popular, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_ip_popular_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_http_stats){
				auto outname = std::string(plcdn_la_opt.output_file_http_stats) +
						parse_nginx_output_filename(plcdn_la_opt.format_http_stats, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_http_stats_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_ip_slowfast){
				auto outname = std::string(plcdn_la_opt.output_file_ip_slowfast) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_slowfast, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_ip_slowfast_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_cutip_slowfast){
				auto outname = std::string(plcdn_la_opt.output_file_cutip_slowfast) +
						parse_nginx_output_filename(plcdn_la_opt.format_cutip_slowfast, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_cutip_slowfast_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if( plcdn_la_opt.output_file_ip_source) {
				auto outname = std::string( plcdn_la_opt.output_file_ip_source) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_source, item.first.c_str("%Y%m%d%H%M"), site_id, user_id);
				auto stream = append_stream(filemap, outname);
				if(stream)
					print_ip_source_table(stream, item.first, item.second, site_id, user_id, n);
			}
		}
	}
	for(auto & it : filemap){
		if(it.second)
			fclose(it.second);
	}
	if(n != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, n);
	return 0;

}
