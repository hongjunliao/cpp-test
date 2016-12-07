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
			std::string surl(url.url, url.end);
			char domain[128];
//			printf("%s: ___url = %s____\n", __FUNCTION__, surl.c_str());
			auto r = parse_domain_from_url(surl.c_str(), domain);
			if(r == 0){
//				printf("%s: ___url = %s, domain = %s____\n", __FUNCTION__, surl.c_str(), domain);
				int site_id, user_id;
				find_site_id(g_sitelist, domain, site_id, &user_id);
				slog._site_id = site_id;
				slog._user_id = user_id;

				slog._url = surl;
				slog._domain = domain;
			}
		}
	}
}

/*try to get connection info from srs_sid_file*/
static void parse_srs_sid_from_file(srs_sid_log & slog, char const * file)
{
	auto f = fopen(file, "r");
	if(!f){
		if(plcdn_la_opt.verbose)
			fprintf(stderr, "%s: fopen '%s' failed\n", __FUNCTION__, file);
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
			char domain[128];
			auto r = parse_domain_from_url(url.url, domain);
			if(r != 0)
				break;	/*FIXME: impossible?*/
			int site_id, user_id;
			find_site_id(g_sitelist, domain, site_id, &user_id);
			slog._site_id = site_id;
			slog._user_id = user_id;
			slog._url = url.url;
			slog._domain = domain;

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
		if(!slog){
			fprintf(stderr, "%s: sid '%d' NOT found\n", __FUNCTION__, sid);
		}
	}
}

/*
 * @param srs_sid_dir: log_dir by srs_sid
 */
void split_srs_log_by_sid(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<int, srs_sid_log> & slogs)
{
	for(char * p = start_p, * q = p; q != start_p + logfile_stat.st_size; ++q){
		if(*q != '\n')
			continue;
		*q = '\0';

		srs_raw_log_t rlog;
		rlog.type = 0;
		rlog.first = p;
		rlog.second = q;
//		printf("___%s____\n", rlog.first);

		auto sid = parse_srs_log_header_sid(rlog.first);
		if(sid < 0){
			if(plcdn_la_opt.verbose)
				fprintf(stderr, "%s: parse sid failed from '%s', skip", __FUNCTION__, rlog.first);
			continue;
		}
		slogs[sid]._logs.push_back(rlog);
		p = q + 1;
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
	for(auto & item : slogs){
		auto & slog = item.second;
		if(!slog){
//			if(plcdn_la_opt.verbose)
//				fprintf(stderr, "%s: 'if(srs_sid_log)' failed for sid '%d', skip\n", __FUNCTION__, item.first);
			continue;
		}
		auto & dstat = logstats[slog._domain];
		do_srs_log_sid_stats(item.first, slog, dstat);
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
				auto ret = boost::filesystem::create_directories(dirname);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
