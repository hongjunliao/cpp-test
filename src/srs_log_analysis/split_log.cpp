/*!
 * This file is PART of plcdn_log_analysis
 * split srs log
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include "srs_log_analysis.h"	/*srs_sid, ...*/
#include "test_options.h"	/*plcdn_la_options*/
#include <sys/stat.h>	/*fstat*/
#include <stdio.h>
#include <vector>		/*std::vector*/
#include <set>			/*std::set*/
#include <time.h>		/*tm*/
#include <string> 		/*std::string*/
#include <unordered_map> 		/*std::unordered_map*/
#include <boost/filesystem.hpp> /*create_directories*/

/*total lines for srs log file*/
size_t g_srs_total_line = 0, g_srs_failed_line = 0, g_srs_slog_line = 0, g_srs_trans_line = 0;
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
/*main.cpp*/
extern std::unordered_map<std::string, site_info> g_sitelist;
/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

static std::string parse_srs_split_filename(char const * fmt,
		char const *interval, char const *day, int site_id, int user_id)
{
	std::unordered_map<std::string, std::string> argmap;
	argmap["interval"] = interval;
	argmap["day"] = day;
	argmap["site_id"] = std::to_string(site_id);
	argmap["user_id"] = std::to_string(user_id);

	std::string outname;
	parse_fmt(fmt, outname, argmap);
	return outname;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void fwrite_srs_sid_log(FILE * f, std::unordered_map<int, std::string> const& sid_log, size_t & n)
{
	for(auto & item : sid_log){
		if(item.second.empty())
			continue;
		auto result = fwrite(item.second.c_str(), sizeof(char), item.second.size(), f);
		if(result < sizeof(item.second.size()) || ferror(f)){
//			fprintf(stderr, "%s: fwrite failed for '%s'\n", __FUNCTION__, item.second.c_str());
			++n;
		}
	}
}

static void fwrite_srs_raw_log(FILE * f, std::vector<srs_raw_log_t> const& logs, size_t & n)
{
	for(auto & item : logs){
		//FIXME: is it possible?
		if(!item.first || !item.second){
			continue;
		}
		auto len = item.second - item.first;
		//FIXME: is it possible?
		if(len <= 0){
			continue;
		}
		char buff[len + 1];	/*endwith \n*/

		/*!
		 * FIXME: @see do_parse_nginx_log_item, I haven't found a better way yet
		 */
		memcpy(buff, item.first, len);	/*@note: NOT strcpy*/
		buff[len] = '\n';
		std::replace(buff, buff + len, '\0', ' ');

//		if(item.type == 4){
//			fprintf(stdout, "%s: _________", __FUNCTION__);
//			for(auto & c : buff)
//				fprintf(stdout, "%c", c);
//			fprintf(stdout, "_________\n");
//		}

		auto result = fwrite(buff, sizeof(char), sizeof(buff), f);
		if(result < sizeof(buff) || ferror(f)){
//			fprintf(stderr, "%s: fwrite failed, result = %zu, len = %zu\n", __FUNCTION__,
//					result, sizeof(buff));
			++n;
		}
	}
}

/*
 * try to write connection info to file
 * @param n: write failed lines
 * */
static void fwrite_srs_sid_to_file(srs_sid_log const & slog, char const * file, size_t & n)
{
//	fprintf(stdout, "%s: write : dir = %s____\n", __FUNCTION__, fullname.c_str());
	auto f = fopen(file, "a");	/*append mode*/
	if(!f){
		if(plcdn_la_opt.verbose)
			fprintf(stderr, "%s: fopen file '%s' for write connection info failed\n", __FUNCTION__, file);
		return;
	}
	for(auto & log : slog._logs){
//		printf("%s: type = %d\n", __FUNCTION__, log.type);
		if(log.type == 1 || log.type == 4){
			auto len = log.second - log.first;
			if(len <= 0)
				continue;

			char buff[len + 1];	/*endwith \n*/
			memcpy(buff, log.first, len);	/*@note: NOT strcpy*/
			buff[len] = '\n';
			std::replace(buff, buff + len, '\0', ' ');
//			printf("%s: connection info = %s, len = %ld\n", __FUNCTION__, buff, len);
			auto result = fwrite(buff, sizeof(char), sizeof(buff), f);
			if(result < sizeof(buff) || ferror(f))
				++n;
		}
	}
	fclose(f);
}

/*try to get connection info from raw logs, it's OK if not found*/
static void parse_srs_sid_from_raw_logs(srs_sid_log & slog)
{
	for(auto & rlog : slog._logs){
		srs_connect_ip ip;
		srs_connect_url url;
		int t = 0;
		auto ret = parse_srs_log_item_conn(rlog.first, ip, url, t);
		if(ret != 0 || t == 0)
			continue;
		if(t == 1){
//				printf("%s: ___ip = %u____\n", __FUNCTION__, ip.ip);
			rlog.type = 1;
			slog._ip = ip.ip;
		}
		else if(t == 2) {
			rlog.type = 4;
			int site_id = 0, user_id = 0;
			std::string sdomain{url.domain, url.d_end};
			find_site_id(g_sitelist, sdomain.c_str(), site_id, &user_id);
			slog._site_id = site_id;
			slog._user_id = user_id;

			slog._url.assign(url.url, url.end);
			slog._domain = sdomain;
		}
	}
}

/*try to get connection info from srs_sid_file*/
static void parse_srs_sid_from_file(srs_sid_log & slog, char const * file)
{
	auto f = fopen(file, "r");
	if(!f){
//		if(plcdn_la_opt.verbose)
//			fprintf(stderr, "%s: fopen '%s' failed\n", __FUNCTION__, file);
		return;
	}
	srs_connect_ip ip;
	srs_connect_url url;
	char buff[6000];
	while(fgets(buff, sizeof(buff), f)){
		int t = 0;
		auto ret = parse_srs_log_item_conn(buff, ip, url, t);
		if(ret != 0)
			continue;
		if(t == 1){
			slog._ip = ip.ip;

			/*@see fwrite_srs_sid_log*/
			srs_raw_log_t rlog;
			rlog.type = 1;
			rlog.first = rlog.second = NULL;
			rlog.buff = buff;
			slog._logs.push_back(rlog);	/*TODO: emplace_back?*/
		}
		else if(t == 2) {
			int site_id, user_id;
			std::string sdomain{url.domain, url.d_end};
			find_site_id(g_sitelist, sdomain.c_str(), site_id, &user_id);
			slog._site_id = site_id;
			slog._user_id = user_id;
			slog._url.assign(url.url, url.end);
			slog._domain = sdomain;

			/*@see fwrite_srs_sid_log*/
			srs_raw_log_t rlog;
			rlog.type = 4;
			rlog.first = rlog.second = NULL;
			rlog.buff = buff;
			slog._logs.push_back(rlog);	/*TODO: emplace_back?*/
		}
	}
	fclose(f);
}

/*get srs_sids from directory @param srs_sid_dir, or write to it*/
void sync_srs_sids_dir(std::unordered_map<int, srs_sid_log> & slogs,
		char const * srs_sid_dir)
{
//	fprintf(stdout, "%s: ___size = %zu, dir = %s____\n", __FUNCTION__, slogs.size(), srs_sid_dir);
	if(!srs_sid_dir || srs_sid_dir[0] == '\0')
		return;
	std::vector<int> sidvec;
	for(auto & item : slogs){
		auto sid = item.first;
		auto & slog = item.second;

		auto && fullname = std::string(srs_sid_dir) + std::to_string(sid);
//		fprintf(stdout, "%s: dir = %s____\n", __FUNCTION__, fullname.c_str());

		if(!slog)
			parse_srs_sid_from_raw_logs(slog);	/*sid NOT exist, first find in log*/
		if(slog){	/*if found, cache them to files*/
			size_t n = 0;
			fwrite_srs_sid_to_file(slog, fullname.c_str(), n);
			if(n != 0)
				fprintf(stderr, "%s: sid = '%d', skipped lines = %zu\n", __FUNCTION__, sid, n);
			continue;
		}
		/*else find sid from file*/
		parse_srs_sid_from_file(slog, fullname.c_str());
		if(!slog)
			sidvec.push_back(sid);
	}
	if(plcdn_la_opt.verbose > 2 && !sidvec.empty()){
		fprintf(stderr, "%s: sid NOT found from srs_sid_log: [", __FUNCTION__);
		for(auto & sid_item : sidvec)
			fprintf(stderr, "%d,", sid_item);
		fprintf(stderr, "]\n");
	}
}

/*
 * @param srs_sid_dir: log_dir by srs_sid
 */
void split_srs_log_by_sid(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<int, srs_sid_log> & slogs)
{
	size_t &total_line = g_srs_total_line, &failed_line = g_srs_failed_line;
	for(char * p = start_p, * q = p; q != start_p + logfile_stat.st_size; ++q){
		if(*q != '\n')
			continue;
		++total_line;
		*q = '\0';

		srs_raw_log_t rlog;
		rlog.type = 0;
		rlog.first = p;
		rlog.second = q;
		p = q + 1;

//		fprintf(stdout, "%s: ____", __FUNCTION__);
//		for(auto p = rlog.first; p != rlog.second; ++p){
//			fprintf(stdout, "%c", *p);
//		}
//		fprintf(stdout, "____\n");

		auto sid = parse_srs_log_header_sid(rlog.first, rlog.second);
		if(sid < 0){
			++failed_line;
			if(plcdn_la_opt.verbose)
				fprintf(stderr, "%s: parse sid failed from '%s', skip\n", __FUNCTION__, rlog.first);
			continue;
		}
		slogs[sid]._logs.push_back(rlog);
	}
	if(plcdn_la_opt.verbose){
		fprintf(stdout, "%s: processed total = %zu, failed = %zu\n", __FUNCTION__, total_line, failed_line);
	}
//	for(auto & item : slogs){
//		fprintf(stdout, "%s: ___sid = %d, size = %zu____\n", __FUNCTION__, item.first, item.second._logs.size());
//		for(auto & log : item.second._logs){
//			fprintf(stdout, "\t____");
//			for(auto p = log.first; p != log.second; ++p){
//				*p? fprintf(stdout, "%c", *p) : fprintf(stdout, "%s", "\\0");
//			}
//			fprintf(stdout, "____\n");
//		}
//	}
}

int parse_srs_log(std::unordered_map<int, srs_sid_log> & slogs,
		std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	size_t & total_line = g_srs_slog_line, & failed_line = g_srs_failed_line, & trans_line = g_srs_trans_line;
	std::vector<int> skipped_sids;	/* sids skipped */
	std::vector<int> skipped_trans;	/*  skipped sids for countof(srs_trans) < 2*/
	for(auto & item : slogs){
		auto & slog = item.second;
		if(!slog){
			skipped_sids.push_back(item.first);
			continue;
		}
		total_line += slog._logs.size();
		auto & dstat = logstats[slog._domain];
		bool skip;
		do_srs_log_sid_stats(item.first, slog, dstat, failed_line, trans_line, skip);
		if(skip)
			skipped_trans.push_back(item.first);
	}
	if(plcdn_la_opt.verbose > 2 && !skipped_sids.empty()){
		fprintf(stderr, "%s: skipped sids because of incomplete: [", __FUNCTION__);
		for(auto & sid_item : skipped_sids)
			fprintf(stderr, "%d,", sid_item);
		fprintf(stderr, "]\n");
	}
	if(plcdn_la_opt.verbose > 2 && !skipped_trans.empty()){
		fprintf(stderr, "%s: skipped sids because of trans < 2: [", __FUNCTION__);
		for(auto & sid_item : skipped_trans)
			fprintf(stderr, "%d,", sid_item);
		fprintf(stderr, "]\n");
	}
	if(plcdn_la_opt.verbose){
		fprintf(stdout, "%s: slog = %zu, failed = %zu, trans = %zu\n", __FUNCTION__,
				total_line, failed_line, trans_line);
	}
	return 0;
}

int split_srs_log(std::unordered_map<std::string, srs_domain_stat> const & logstats,
		char const * folder, char const * fmt)
{
	if(!folder || folder[0] == '\0' || !fmt || fmt[0] == '\0')
		return -1;
	size_t n = 0;
	std::unordered_map<std::string, FILE *> filemap; /*srs_log_dir: log_file*/
	for(auto & dstat : logstats){
		auto site_id = dstat.second._site_id, user_id = dstat.second._user_id;
		auto & sid_log = dstat.second._sid_log;
		for(auto & item : dstat.second._stats){
			char buft1[32], buft2[32];
			auto && fname = parse_srs_split_filename(fmt,
					item.first.c_str_r(buft1, sizeof(buft1)), item.first.c_str_r(buft2, sizeof(buft2), "%Y%m%d")
					, site_id, user_id);
			auto && fullname = (std::string(folder) + fname);

			auto len = fullname.size();
			char dirname[len + 1];
			strncpy(dirname, fullname.c_str(), len);
			dirname[len] = '\0';
			auto c = strrchr(dirname, '/');
			if(c){
				*c = '\0';
				boost::system::error_code ec;
				auto ret = boost::filesystem::create_directories(dirname, ec);
			}
			auto & file = filemap[fname];
			if(!file){
				file = fopen(fullname.c_str(), "a");
				if(!file){
					if(plcdn_la_opt.verbose)
						fprintf(stderr, "%s: fopen '%s' failed, skip\n", __FUNCTION__, fullname.c_str());
					/*FIXME n++?*/
					continue;
				}
				/*if this file is first open, append sid_log first*/
				fwrite_srs_sid_log(file, sid_log, n);
			}
			fwrite_srs_raw_log(file, item.second.logs, n);
		}
	}
	if(n != 0)
		fprintf(stderr, "%s: skipped lines = %zu\n", __FUNCTION__, n);
	for(auto & it : filemap) {
		if(it.second)
			fclose(it.second);
	}
	return 0;
}

/*write srs log by sid, usually for debug*/
int fwrite_srs_log_by_sid(std::unordered_map<int, srs_sid_log> & slogs, char const * folder)
{
	if(!folder || folder[0] == '\0')
		return -1;
	size_t n = 0;
	for(auto & item : slogs){
		auto & sid = item.first;
		auto & slog = item.second;

		char buff[strlen(folder) + 20 + 1];
		snprintf(buff, sizeof(buff) - 1, "%s/%d", folder, sid);
		buff[sizeof(buff) - 1] = '\0';

		auto f = fopen(buff, "a");
		if(!f){
			fprintf(stderr, "%s: append sid log for sid '%d' failed\n", __FUNCTION__, sid);
			continue;
		}
		for(auto & rlog : slog._logs){
			auto len = rlog.second - rlog.first + 1;
			if(len <= 0) continue;

			/*FIXME: ugly but worked*/
			auto & c = rlog.second;
			auto old_c = *c;
			*c = '\n';

			auto result = fwrite(rlog.first, sizeof(char), len, f);
			*c = old_c;

			if(result < (size_t)len || ferror(f)){
				++n;
			}
		}
		fclose(f);
	}
	if(n != 0)
		fprintf(stderr, "%s: failed lines = %zu\n", __FUNCTION__, n);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
