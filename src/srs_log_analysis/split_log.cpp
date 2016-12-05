/*!
 * This file is PART of plcdn_log_analysis
 * split srs log
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include "srs_log_analysis.h"	/*srs_sid, ...*/
#include <sys/stat.h>	/*fstat*/
#include <stdio.h>
#include <vector>		/*std::vector*/
#include <time.h>		/*tm*/
#include <string> 		/*std::string*/
#include <unordered_map> 		/*std::unordered_map*/
#include <boost/filesystem.hpp> /*create_directories*/

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
static void fwrite_srs_sid_log(FILE * f,
		std::vector<srs_raw_log_t> const & logs, size_t & n)
{
//	printf("%s: size = %zu____\n", __FUNCTION__, logs.size());
	for(auto & log : logs){
//		printf("%s: type = %d\n", __FUNCTION__, log.type);
		if(log.type == 1 || log.type == 4){
			auto buff = log.first;
			auto len = log.second - log.first;
//			printf("%s: connection info = %s, len = %ld\n", __FUNCTION__, buff, len);
			auto result = fwrite(log.first, sizeof(char), len, f);
			/*FIXME*/
			fwrite("\n", sizeof(char), 1, f);

			if(result < sizeof(buff) || ferror(f))
				++n;
		}
	}
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
		if(slog){
//			fprintf(stdout, "%s: write : dir = %s____\n", __FUNCTION__, fullname.c_str());
			/*sid exist, write to file*/
			auto f = fopen(fullname.c_str(), "a");	/*append mode*/
			if(!f){
				fprintf(stderr, "%s: write connection info for sid '%d' in '%s' failed, skip\n",
									__FUNCTION__, sid, fullname.c_str());
				continue;
			}
			size_t n = 0;
			fwrite_srs_sid_log(f, slog._logs, n);
			if(n != 0)
				fprintf(stderr, "%s: write connection info for sid '%d', skiped lines = %zu\n",
								__FUNCTION__, sid, n);
			fclose(f);
			continue;
		}
		/*sid NOT exist, read from file*/
		auto f = fopen(fullname.c_str(), "r");
		if(!f){
			fprintf(stderr, "%s: connection info looking up from '%s' NOT found, skip\n",
								__FUNCTION__, fullname.c_str());
			continue;
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
			}
		}
		fclose(f);
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

		p = q + 1;

		auto sid = parse_srs_log_header_sid(rlog.first);
		if(sid < 0)
			continue;

		auto & slog = slogs[sid];
		if(slog){
//			printf("%s: ___sid ok: site_id = %d, ip = %u____\n", __FUNCTION__, slog._site_id, slog._ip);
			continue;
		}
		/*try to get connection info, it's OK if not found*/
		srs_connect_ip ip;
		srs_connect_url url;
		int t = 0;
		auto ret = parse_srs_log_item_conn(rlog.first, ip, url, t);
		if(ret != 0 || t == 0) continue;

		if(t == 1){
//			printf("%s: ___[%s], found connection info: %d, type=%d, ip = %u____\n",
//					__FUNCTION__, rlog.first, sid, t, ip.ip);
			rlog.type = 1;
			slog._ip = ip.ip;
		}
		else if(t == 2) {
//			printf("%s: ___[%s], found connection info: %d, type=%d, url = %s____\n",
//					__FUNCTION__, rlog.first, sid, t, url.url);
			rlog.type = 4;
			char domain[128];
			auto r = parse_domain_from_url(url.url, domain);
			if(r == 0){
//				printf("%s: ___url = %s, domain = %s____\n", __FUNCTION__, url.url, domain);

				int site_id, user_id;
				find_site_id(g_sitelist, domain, site_id, &user_id);
				slog._site_id = site_id;
				slog._user_id = user_id;
				slog._url = url.url;
				slog._domain = domain;
			}
		}
		slog._logs.push_back(rlog);
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
		for(auto p = buff; p + 1 != buff + len; ++p){
			if(*p == '\0' && *(p + 1) == ' '){
				*p = '"';
			}
		}
		buff[len] = '\n';
		std::replace(buff, buff + len, '\0', ' ');

		auto result = fwrite(buff, sizeof(char), sizeof(buff), f);
		if(result < sizeof(buff) || ferror(f))
			++n;
	}
}

int parse_srs_log(std::unordered_map<int, srs_sid_log>  const & slogs,
		std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	for(auto & item : slogs){
		srs_sid_log const & slog = item.second;
		if(!slog){
			continue;
		}
		srs_domain_stat & dstat = logstats[slog._domain];
		do_srs_log_sid_stats(item.first, slog, dstat);
	}
	return 0;
}

int split_srs_log(std::unordered_map<std::string, srs_domain_stat> const & logstats,
		char const * folder, char const * fmt)
{
	size_t n = 0;
	/*srs_log_dir: log_file*/
	std::unordered_map<std::string, FILE *> filemap;
	for(auto & dstat : logstats){
		for(auto const& item : dstat.second._stats){
			auto site_id = dstat.second._site_id, user_id = dstat.second._user_id;
			char buft1[32], buft2[32];
			auto && fname = parse_srs_split_filename(fmt,
					item.first.c_str_r(buft1, sizeof(buft1)), item.first.c_str_r(buft2, sizeof(buft2), "%Y%m%d")
					, site_id, user_id);
			auto && fullname = (std::string(folder) + fname);

			char dirname[fullname.size() + 1];
			strncpy(dirname, fullname.c_str(), sizeof(dirname));

			auto c = strrchr(dirname, '/');
			if(c){
				*c = '\0';
				auto ret = boost::filesystem::create_directories(dirname);
			}
			auto & file = filemap[fname];
			if(!file)
				file = fopen(fullname.c_str(), "a");
			if(!file){
				fprintf(stderr, "%s: fopen failed: '%s', skip\n", __FUNCTION__, fullname.c_str());
				/*FIXME n++?*/
				continue;
			}
			fwrite_srs_raw_log(file, item.second._logs, n);
		}
		for(auto & it : filemap) {
			if(it.second)
				fclose(it.second);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
