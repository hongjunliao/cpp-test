/*!
 * This file is PART of srs_log_analysis
 * ouput results table
 */
#include "srs_log_analysis.h"	/*srs_trans, ...*/
#include <stdio.h>
#include <string.h> 	/*strcmp*/
#include <vector>		/*std::vector*/
#include <algorithm>	/*std::find_if*/
#include <time.h>		/*strftime*/
#include "net_util.h"	/*netutil_get_ip_str*/
#include "test_options.h"		/*plcdn_la_options*/

////////////////////////////////////////////////////////////////////////////////////////////////
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
/*main.cpp*/
extern time_t g_plcdn_la_start_time;
extern int g_plcdn_la_device_id;
/*nginx_log_analysis/parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);
/*  merge_srs_flow.cpp */
extern int merge_srs_flow_same_datetime(FILE *& f);

static FILE * & get_stream_by_filename(std::map<std::string, FILE *> & filemap, std::string const& filename);
static std::string parse_srs_output_filename(char const * fmt, char const *interval, char const *day, int site_id, int user_id);

////////////////////////////////////////////////////////////////////////////////////////////////
static FILE * & get_stream_by_filename(std::map<std::string, FILE *> & filemap, std::string const& filename)
{
	if(!filemap[filename])
		filemap[filename] = fopen(filename.c_str(), "a+");	/*append extended mode*/
	return filemap[filename];
}

static std::string parse_srs_output_filename(char const * fmt, char const *interval, char const *day, int site_id, int user_id)
{
	std::unordered_map<std::string, std::string> argmap;
	char buff[32] = "";
	strftime(buff, sizeof(buff), "%Y%m%d%H%M", localtime(&g_plcdn_la_start_time));

	argmap["device_id"] = std::to_string(g_plcdn_la_device_id);
	argmap["datetime"] = buff;
	argmap["day"] = day;
	argmap["interval"] = interval;
	argmap["site_id"] = std::to_string(site_id);
	argmap["user_id"] = std::to_string(user_id);

	std::string outname;
	parse_fmt(fmt, outname, argmap);
	return outname;
}

void fprint_srs_log_stats(std::unordered_map<std::string, srs_domain_stat> const& srs_stats)
{
//	if(plcdn_la_opt.verbose)
//		fprintf(stream, "%-8s%-13s%-16s%-40s%-14s%-14s\n", "site_id", "time", "client_ip", "url", "obytes", "ibytes");
		//%-8d%-13s%-16s%-40s%-14zu%-14zu\n
	size_t n = 0;
	std::map<std::string, FILE *> filemap; /*for output filenames*/
	for(auto const& dstat_it : srs_stats){
		auto site_id = dstat_it.second._site_id;
		auto user_id = dstat_it.second._user_id;
		for(auto const& stat_it : dstat_it.second._stats){
			auto & stat = stat_it.second;
			char buft[32], buft2[32];;
			if(plcdn_la_opt.output_srs_flow){
				auto outname = std::string(plcdn_la_opt.output_srs_flow) +
						parse_srs_output_filename(plcdn_la_opt.format_srs_flow,
								stat_it.first.c_str_r(buft, sizeof(buft)),
								stat_it.first.c_str_r(buft2, sizeof(buft2), "%Y%m%d"),
								site_id, user_id);
				auto stream = get_stream_by_filename(filemap, outname);
				if(!stream)
					continue;
				/*flow stats, format: 'site_id datetime device_id obytes ibytes obps ibps user_id'*/
				auto obytes = stat.obytes_total(), ibytes = stat.ibytes_total();
				auto obps = obytes * 1.0 * 8 / plcdn_la_opt.interval,
						ibps = ibytes * 1.0 * 8 / plcdn_la_opt.interval;
				auto sz = fprintf(stream, "%d %s %d %zu %zu %.0f %.0f %d\n", site_id, buft, g_plcdn_la_device_id,
						obytes, ibytes, obps, ibps, user_id);
				if(sz <= 0) ++n;
			}
		}
	}
	for(auto & it : filemap){
		if(it.second){
			/* merge rows in srs_flow_table where datetime same*/
			if(plcdn_la_opt.srs_flow_merge_same_datetime == 1){
				std::fseek(it.second, 0, SEEK_SET);	/* move to start */
				auto r = merge_srs_flow_same_datetime(it.second);
				if(r != 0 && plcdn_la_opt.verbose > 4){
					fprintf(stderr, "%s: merge_srs_flow_same_datetime failed\n", __FUNCTION__);
				}
			}
			fclose(it.second);
		}
	}
	//url stat
//	for(auto & url : stat.urls){
//		auto sid = url.first;
//		char buf[32];
//		netutil_get_ip_str(stat.ips.at(sid), buf, sizeof(buf));
//		/*format: 'site_id time client_ip url obytes ibytes'*/
//		fprintf(stream, "%d %s %s %s %zu %zu\n",
//				site_id
//				, buft
//				, buf
//				, url.second.c_str()
//				, stat.obytes.at(sid)/*(s.obytes.count(url.first) != 0? s.obytes.at(url.first) : 0)*/
//				, stat.ibytes.at(sid)/*(s.ibytes.count(url.first) != 0? s.ibytes.at(url.first) : 0)*/
//				);
//
//	}

}
