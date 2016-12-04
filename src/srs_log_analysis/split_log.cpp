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
/*get srs_sids from directory @param srs_sid_dir*/
static void get_srs_sids_from_dir(std::unordered_map<srs_sid, std::vector<srs_raw_log_t>> & slogs,
		char const * srs_sid_dir)
{
	for(auto it = slogs.begin(); it != slogs.end(); ++it){
		auto sid = it->first;
		if(sid)
			continue;
		auto dir = std::string(srs_sid_dir) + std::to_string(sid._sid);
		auto * f = fopen(dir.c_str(), "r");
		if(!f){
			fprintf(stderr, "%s: connection info for sid '%d' in '%s' NOT found, skip\n",
					__FUNCTION__, sid._sid, dir.c_str());
			continue;
		}
		srs_connect_ip ip;
		srs_connect_url url;
		bool is_ip_found = false, is_url_found = false;
		char buff[6000];
		while(fgets(buff, sizeof(buff), f)){
			if(is_ip_found && is_url_found)
				break;
			int t = 0;
			auto ret = parse_srs_log_item_conn(buff, '\n', ip, url, t);
			if(ret != 0)
				continue;
			if(t == 1){
				sid._ip = ip.ip;
				is_ip_found = true;
			}
			else if(t == 2) {
				char domain[128];
				auto r = parse_domain_from_url(url.url, domain, sizeof(domain));
				if(r != 0)
					break;	/*FIXME: impossible?*/
				is_url_found = true;
				int site_id, user_id;
				find_site_id(g_sitelist, domain, site_id, &user_id);
				sid._site_id = site_id;
				sid._user_id = user_id;
			}
		}
	}
}

/* FIXME: std::unordered_map<srs_sid, std::vector<srs_raw_log_t>> order by srs_sid::_time
 * @date 2016/12/03
 *
 * @param srs_sid_dir: log_dir by srs_sid
 */
void split_srs_log_by_sid(char * start_p, struct stat const & logfile_stat, char const * srs_sid_dir,
		std::unordered_map<srs_sid, std::vector<srs_raw_log_t>> & slogs)
{
	srs_raw_log_t rlog;
	for(char * p = start_p, * q = p; q != start_p + logfile_stat.st_size; ++q){
		if(*q == '\n'){
			rlog.first = p;
			rlog.second = q;
			auto sid = parse_srs_log_header_sid(p);
			if(sid < 0)
				continue;

			srs_sid k(sid);
			srs_connect_ip ip;
			srs_connect_url url;
			int t = 0;
			auto ret = parse_srs_log_item_conn(rlog.first, rlog.second, ip, url, t);
			if(ret == 0){
				if(t == 1)
					k._ip = ip.ip;
				else if(t == 2) {
					char domain[128];
					auto r = parse_domain_from_url(url.url, domain, sizeof(domain));
					if(r == 0){
						int site_id, user_id;
						find_site_id(g_sitelist, domain, site_id, &user_id);
						k._site_id = site_id;
						k._user_id = user_id;
					}
				}
			}
			slogs[k].push_back(rlog);

			p = q + 1;
		}
	}
	get_srs_sids_from_dir(slogs, srs_sid_dir);
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

/*!
 * append logs from @param slog to dir @param logdir, by sid
 * because of append, connection log preserved
 */
static void append_srs_log_by_sid(std::unordered_map<srs_sid, std::vector<srs_raw_log_t>> const& slog,
		char const * folder, char const * fmt, size_t& n)
{
	/*sid : log_file*/
	std::unordered_map<std::string, FILE *> filemap;
	for(auto & it : slog){
		auto sid = it.first;
		char buft1[32], buft2[32];
		time_group t(sid._time);
		auto && fname = parse_srs_split_filename(fmt,
				t.c_str_r(buft1, sizeof(buft1)), t.c_str_r(buft2, sizeof(buft2), "%Y%m%d")
				, sid._site_id, sid._user_id);
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
		fwrite_srs_raw_log(file, it.second, n);
	}
	for(auto & it : filemap) {
		if(it.second)
			fclose(it.second);
	}
}

int parse_srs_log(std::unordered_map<srs_sid, std::vector<srs_raw_log_t>> const & slogs,
		std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	size_t failed_lines = 0;
//	append_srs_log_by_sid(slogs, logdir, fmt, failed_lines);
//
//	for(auto & it : slog){
//		auto && dir = logdir + std::to_string(it.first);
//		auto f = fopen(dir.c_str(), "r");
//		if(!f) {
//			fprintf(stdout, "%s: can't open srs_sid_log_file '%s' for read\n", __FUNCTION__, dir.c_str());
//			continue;
//		}
//	}

	return 0;
}

int split_srs_log(std::unordered_map<std::string, srs_domain_stat> const & logstats,
		char const * folder, char const * fmt)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
