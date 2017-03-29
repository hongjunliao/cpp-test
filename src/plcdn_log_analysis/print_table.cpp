/*!
 * This file is PART of plcdn_log_analysis
 * ouput results table
 * @author hongjun.liao <docici@126.com>
 * @date 2016/9
 */
#include <stdio.h>
#include <ctime>		/* std::difftime */
#include <limits.h>		/*PATH_MAX*/
#include <string.h> 	/*strncpy*/
#include <netinet/in.h>
#include <map>			/*std::map*/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "test_options.h"		/*plcdn_la_options*/
#include "string_util.h"		/*sha1sum*/
#include "net_util.h"	/*netutil_get_ip_str*/
#include <algorithm>	/*std::sort*/
/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
/*main.cpp*/
extern time_t g_plcdn_la_start_time;
extern int g_plcdn_la_device_id;

/* nginx_rotate.cpp */
extern time_t g_nginx_rotate_time;

static std::string parse_nginx_output_filename(char const * fmt, char const *interval, int site_id, int user_id);
/*flow table*/
static void print_flow_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*hot url*/
static void print_url_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*hot ip*/
static void print_ip_popular_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*httpstatus_statistics*/
static void print_http_stats_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*ip_slowfast, @param topn, print ONLY top n*/
static void print_ip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n, int topn = 20);
/*cutip_slowfast*/
static void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n);
static void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n, int topn);

/*url_key*/
//static void print_url_key_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);
/*ip_source*/
static void print_url_key_table(FILE * stream, int site_id, int user_id,nginx_log_stat const& stat,
		std::unordered_map<std::string, std::string> const & urlkey, size_t& n);

static void print_ip_source_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n);

/* merge tables for same datetime */
/*  merge_srs_flow.cpp */
extern int merge_nginx_flow_datetime(FILE *& f);
extern int merge_nginx_url_popular_datetime(FILE *& f);
extern int merge_nginx_ip_popular_datetime(FILE *& f);
extern int merge_nginx_http_stats_datetime(FILE *& f);
extern int merge_nginx_ip_source_datetime(FILE *& f);
extern int merge_nginx_cutip_slowfast_datetime(FILE *& f);
extern int merge_nginx_ip_slowfast_datetime(FILE *& f);
extern int merge_nginx_url_key(FILE *& f);

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
	/* @NOTES: @date 2017/02/16:
	 * add 2 new fields: tx_rtmp_in(srs ibytes), tx_rtmp_out(srs obytes) @see append_flow_nginx
	 *
	 * @NOTES: @date 2017/03//10:
	 * add 4 new fields: loc, isp, fst_pkg_time, svg_speed. for user/yunduan,
	 * 1.see https://www.isurecloud.com/
	 * 2.see plcdn_log_transform.cpp
	 * */
	char buft[32] = "(error)";
	g.c_str_r(buft, sizeof(buft));
	for(auto const& li_item : stat._locisp_stats){
		auto const & li = li_item.first;
		auto const & listat = li_item.second;

		size_t access = 0, bytes = 0, access_m = 0, bytes_m = 0;
		locisp_stat_access_bytes(listat, access, bytes, access_m, bytes_m);

		char loc_isp[7 + 1 + 3 + 1 + 20] = "- -"; /*<loc><blank><isp><NULL><?>*/
		/* format: site_id, datetime, device_id, num_total, bytes_total, user_id, pvs_m, px_m,
		 * tx_rtmp_in, tx_rtmp_out,
		 * loc, isp, fst_pkg_time, svg_speed */
		auto sz = fprintf(stream, "%d %s %d %zu %zu %d %zu %zu %zu %zu %s %.0f %.0f\n",
					site_id, buft, g_plcdn_la_device_id, access
					, bytes, user_id, access_m, bytes_m
					, stat.srs_in, stat.srs_out
					, li.loc_isp_c_str(loc_isp, sizeof(loc_isp)), locisp_stat_fpt(listat), locisp_stat_svg(listat));
		if(sz <= 0) ++n;
	}
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
		/* format:
		 * datetime(deleted) url_key, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
		 * , num_403, num_404, num_416, num_499, num_500, num_502, num_other
		 *
		 * NOTE: datetime deleted, @date 2017/03/24 @author hongjun.liao <docici@126.com> */
		auto sz = fprintf(stream, "%s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
					url.c_str()
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
		if(plcdn_la_opt.min_ip_popular > 0 && ipstat.access < (size_t)plcdn_la_opt.min_ip_popular)
			continue;
		if(plcdn_la_opt.max_ip_popular >= 0 && ipstat.access > (size_t)plcdn_la_opt.max_ip_popular)
			continue;

		char ipbuff[20] = "0.0.0.0";
		char buft[32];
		/*format: site_id, device_id, ip, datetime, num*/
		/*FIXME: ip is string format?*/
		auto sz = fprintf(stream, "%d %d %s %s %zu\n",
						site_id, g_plcdn_la_device_id, netutil_get_ip_str(ip_item.first, ipbuff, sizeof(ipbuff)),
						g.c_str_r(buft, sizeof(buft)), ipstat.access);
		if(sz <= 0) ++n;
	}
}

inline void print_http_stats_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{

	char buft[32] = "(error)";
	g.c_str_r(buft, sizeof(buft));
	for(auto const& li_item : stat._locisp_stats){
		auto const & li = li_item.first;
		auto const & listat = li_item.second;

		char loc_isp[7 + 1 + 3 + 1 + 20] = "- -"; /*<loc><blank><isp><NULL><?>*/
		li.loc_isp_c_str(loc_isp, sizeof(loc_isp));

		for(auto & c_item : listat._code){
			auto & cstat = c_item.second;
			/* NOTE: num_m, loc, isp are new added for user/yunduan, @author hongjun.liao <docici@126.com> @date 2017/03 */
			/* format: site_id, device_id, httpstatus, datetime, num, num_m, loc, isp */
			auto sz = fprintf(stream, "%d %d %d %s %zu %zu %s\n",
					site_id, g_plcdn_la_device_id, c_item.first, buft, cstat.access, cstat.access_m, loc_isp);
			if(sz <= 0) ++n;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
/*@see ip_stat, print_ip_slowfast_table*/
struct ip_speed
{
	uint32_t ip;
	size_t bytes; 	/*bytes total*/
	size_t sec;		/*time total, in seconds*/
	double speed;
};

/*@see print_ip_slowfast_table*/
static ip_speed calc_ip_speed(std::unordered_map<uint32_t, ip_stat>::value_type const& item)
{
	auto & stat = item.second;

	ip_speed ret;
	ret.ip = item.first;
	ret.bytes = stat.bytes;
	ret.sec = stat.sec;
	ret.speed = (stat.sec != 0? (double)stat.bytes / stat.sec : 0.0);

	return ret;
}

/* FIXME: how to compare?, 2016/12/09 */
static bool sort_by_speed(ip_speed const& a, ip_speed const& b)
{
	if(a.sec != 0 && b.sec != 0)
		return a.speed > b.speed;
	if(a.sec != 0 || b.sec != 0)
		return a.sec != 0;
	return a.bytes > b.bytes;
};

inline void print_ip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n, int topn/* = 20*/)
{
//	printf("%s: ______size=%zu, topn=%d_____________\n", __FUNCTION__, stat._ip_stats.size(), topn);
	/* FIXME: print only top 20?, 2016/12/09
	 * ~/yong.lu/nginx/path/UASStats$ wc -l *
	 */
	std::vector<ip_speed> vec(stat._ip_stats.size());	/*must set init size*/
	std::transform(stat._ip_stats.cbegin(), stat._ip_stats.cend(), vec.begin(), calc_ip_speed);
	std::sort(vec.begin(), vec.end(), sort_by_speed);

	for(auto const& item : vec){
		/*format: device_id, ip, datetime, speed, type(MISS,HIT)*/
		/*FIXME: type?*/
		char buft[32];
		auto speed = (item.sec != 0? item.speed : (double)item.bytes * 1000000.0);
		auto sz = fprintf(stream, "%d %u %s %.0f %d\n",
				g_plcdn_la_device_id, item.ip, g.c_str_r(buft, sizeof(buft)), speed, 0);
		if(sz <= 0) ++n;

		if(--topn < 1)
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////
/*@see ip_stat, print_cutip_slowfast_table*/
struct cutip_speed
{
	char cutip[16];
	size_t bytes; 	/*bytes total*/
	size_t sec;		/*time total, in seconds*/
	double speed;
};

/*@see print_cutip_slowfast_table*/
static cutip_speed calc_cutip_speed(std::unordered_map<cutip_group, ip_stat>::value_type const& item)
{
	auto & stat = item.second;

	cutip_speed ret;
	strncpy(ret.cutip, item.first.c_str(), sizeof(ret.cutip));
	ret.bytes = stat.bytes;
	ret.sec = stat.sec;
	ret.speed = (stat.sec != 0? (double)stat.bytes / stat.sec : 0.0);

	return ret;
}

/* FIXME: how to compare?, 2016/12/09 */
static bool cutip_sort_by_speed(cutip_speed const& a, cutip_speed const& b)
{
	if(a.sec != 0 && b.sec != 0)
		return a.speed > b.speed;
	if(a.sec != 0 || b.sec != 0)
		return a.sec != 0;
	return a.bytes > b.bytes;
};

inline void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n, int topn)
{
	std::vector<cutip_speed> vec(stat._cuitip_stats.size());	/*must set init size*/
	std::transform(stat._cuitip_stats.cbegin(), stat._cuitip_stats.cend(), vec.begin(), calc_cutip_speed);
	std::sort(vec.begin(), vec.end(), cutip_sort_by_speed);

	for(auto const& item : vec){
		auto speed = (item.sec != 0? item.speed : (double)item.bytes * 1000000.0);
		/*format: device_id, datetime, ip, speed*/
		char buft[32];
		cutip_group cutip(item.cutip);
		auto sz = fprintf(stream, "%d %s %s %.0f\n",
				g_plcdn_la_device_id, g.c_str_r(buft, sizeof(buft)), cutip.c_str(), speed);
		if(sz <= 0) ++n;
		if(--topn < 1)
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////
inline void print_cutip_slowfast_table(FILE * stream, time_group const& g, nginx_log_stat const& stat,
		int site_id, int user_id, size_t& n)
{
	for(auto const& cutip_item : stat._cuitip_stats){
		auto const & cutipstat = cutip_item.second;
		double speed = cutipstat.sec > 0? (double)cutipstat.bytes / cutipstat.sec : cutipstat.bytes * 1000000.0;
		/*format: device_id, datetime, ip, speed*/
		char buft[32];
		auto sz = fprintf(stream, "%d %s %s %.0f\n",
				g_plcdn_la_device_id, g.c_str_r(buft, sizeof(buft)), cutip_item.first.c_str(), speed);
		if(sz <= 0) ++n;
	}
}

static void print_url_key_table(FILE * stream, int site_id, int user_id,nginx_log_stat const& stat,
		std::unordered_map<std::string, std::string> const & urlkey, size_t& n)
{
		for(auto const & url_item : stat._url_stats){
			auto const & url = url_item.first;
			if(urlkey.count(url) == 0)
				continue;	/* this should never happen */
			/* format: 'url_key url' */
			char c = (url_item.second.status == 0? '+' : '-');
			auto sz = fprintf(stream,"%c\"%s\"%s\n", c, url.c_str(),  urlkey.at(url).c_str());
			if(sz <= 0) ++n;
		}
}

void print_ip_source_table(FILE * stream, time_group const& g, nginx_log_stat const& stat, int site_id, int user_id, size_t& n)
{
	for(auto const& li_item : stat._locisp_stats){
		auto const & li = li_item.first;
		char loc_isp[7 + 1 + 3 + 1 + 20]; /*<loc><blank><isp><NULL><?>*/
		li.loc_isp_c_str(loc_isp, sizeof(loc_isp));

		auto const & listat = li_item.second;
		size_t access = 0, bytes = 0, access_m = 0, bytes_m = 0;
		locisp_stat_access_bytes(listat, access, bytes, access_m, bytes_m);
		/*format: bw_time, local_id, isp_id, pvs, tx, pvs_m, tx_m, device_id*/
		char buft[32];
		auto sz = fprintf(stream, "%s %s %zu %zu %zu %zu %d\n",
						g.c_str_r(buft, sizeof(buft)),
						loc_isp,
						access, bytes, access_m, bytes_m, g_plcdn_la_device_id);
		if(sz <= 0) ++n;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int (* merge_nginx_table_fn_t)(FILE *&);

static FILE * & append_stream(std::map<std::string, std::tuple<FILE *, merge_nginx_table_fn_t>> & filemap,
		std::string const& filename,
		merge_nginx_table_fn_t fn = NULL)
{
	auto & val = filemap[filename];
	auto & file = std::get<0>(val);
	if(!file){
		file = fopen(filename.c_str(), "a+");	/*append extended mode*/
		std::get<1>(val) = fn;
	}
	return file;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
int print_nginx_log_stats(std::unordered_map<std::string, nginx_domain_stat> const& stats)
{
	std::map<std::string, std::tuple<FILE *, merge_nginx_table_fn_t>> filemap; /*for output filenames*/
	size_t n = 0;
	for(auto const& dstat : stats){
		auto site_id = dstat.second._site_id, user_id = dstat.second._user_id;
		auto & urlkey = dstat.second._url_key;
		for(auto const& item : dstat.second._stats){
			if(item.second.empty())
				continue;
			char buft[32];
			if(plcdn_la_opt.work_mode == 2 &&
					std::difftime(g_nginx_rotate_time, item.first.t()) > (double)plcdn_la_opt.nginx_rotate_time){
				if(plcdn_la_opt.verbose > 3)
					fprintf(stdout, "%s: WARNING!!! time '%s' expired, skip\n", __FUNCTION__, item.first.c_str_r(buft, sizeof(buft)));
				continue;
			}
			if(plcdn_la_opt.output_nginx_flow){
				auto outname = std::string(plcdn_la_opt.output_nginx_flow) +
						parse_nginx_output_filename(plcdn_la_opt.format_nginx_flow, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_flow_datetime);
				if(stream)
					print_flow_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_url_popular){
				auto outname = std::string(plcdn_la_opt.output_file_url_popular) +
						parse_nginx_output_filename(plcdn_la_opt.format_url_popular, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_url_popular_datetime);
				if(stream)
					print_url_popular_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_ip_popular){
				auto outname = std::string(plcdn_la_opt.output_file_ip_popular) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_popular, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_ip_popular_datetime);
				if(stream)
					print_ip_popular_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_http_stats){
				auto outname = std::string(plcdn_la_opt.output_file_http_stats) +
						parse_nginx_output_filename(plcdn_la_opt.format_http_stats, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_http_stats_datetime);
				if(stream)
					print_http_stats_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_ip_slowfast){
				auto outname = std::string(plcdn_la_opt.output_file_ip_slowfast) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_slowfast, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_ip_slowfast_datetime);
				if(stream)
					print_ip_slowfast_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_cutip_slowfast){
				auto outname = std::string(plcdn_la_opt.output_file_cutip_slowfast) +
						parse_nginx_output_filename(plcdn_la_opt.format_cutip_slowfast, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_cutip_slowfast_datetime);
				if(stream)
					print_cutip_slowfast_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if( plcdn_la_opt.output_file_ip_source) {
				auto outname = std::string( plcdn_la_opt.output_file_ip_source) +
						parse_nginx_output_filename(plcdn_la_opt.format_ip_source, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_ip_source_datetime);
				if(stream)
					print_ip_source_table(stream, item.first, item.second, site_id, user_id, n);
			}
			if(plcdn_la_opt.output_file_url_key){
				auto outname = std::string( plcdn_la_opt.output_file_url_key) +
				parse_nginx_output_filename(plcdn_la_opt.format_file_url_key, item.first.c_str_r(buft, sizeof(buft)), site_id, user_id);
				auto stream = append_stream(filemap, outname, merge_nginx_url_key);
				if(stream)
					print_url_key_table(stream, site_id, user_id, item.second, urlkey, n);
			}
		}
	}
	for(auto & it : filemap){
		auto & f = std::get<0>(it.second);
		auto fn = std::get<1>(it.second);
		if(f){
			/* NOTE: currenlty nginx/url_key_table don't have 'datetime', merge operation should always requrired
			 * @author hongjun.liao <docici@126.com>  @date 2017/03/03 */
			if(fn == merge_nginx_url_key || (!plcdn_la_opt.no_merge_datetime && fn)){
				std::fseek(f, 0, SEEK_SET);	/* move to start */
				auto r = fn(f);
				if(r != 0 && plcdn_la_opt.verbose > 4){
					fprintf(stderr, "%s: merge nginx table failed\n", __FUNCTION__);
				}
			}
			fclose(f);
		}
	}
	if(n != 0 && plcdn_la_opt.verbose)
		fprintf(stderr, "%s: WARNING, skipped %zu lines\n", __FUNCTION__, n);
	return 0;

}
