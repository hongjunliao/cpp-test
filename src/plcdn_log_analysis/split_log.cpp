/*!
 * This file is PART of nginx_log_analysis
 * split nginx log
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include "nginx_log_analysis.h"	/*nginx_domain_stat, ...*/
#include <sys/mman.h>	/*mmap*/
#include <stdio.h>
#include <time.h>		/*tm*/
#include <string> 		/*std::string*/
#include <unordered_map> 		/*std::unordered_map*/
#include <boost/filesystem.hpp> /*create_directories*/

//#include "bd_test.h"			/*test_srs_log_stats_main*/
//#include "test_options.h" 		/*sla_options*/
//
//#include <stdio.h>
//#include <string.h> 	/*strncpy*/
//#include <sys/sysinfo.h>	/*get_nprocs*/
//#include <sys/stat.h>	/*fstat*/
////#include <locale.h> 	/*setlocale*/
//#include <pthread.h> 	/*pthread_create*/
//#include <map>				/*std::map*/
////#include <thread>		/*std::thread*/
////#include <atomic>		/*std::atomic*/
//#include "bd_test.h"		/*test_nginx_log_stats_main*/
//#include "test_options.h"	/*nla_options**/
//#include "srs_log_analysis.h"	/*srs_log_context*/
//#include "string_util.h"	/*md5sum*/
//#include "net_util.h"	/*get_if_addrs, ...*/
//#include <algorithm>	/*std::min*/

/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

static std::string parse_nginx_split_filename(char const * fmt,
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

//FIXME: lost '-' at end of each log, @date 2016/11/23
static void fwrite_nginx_raw_log(FILE * f, std::vector<nginx_raw_log_t> const& logs, size_t & n)
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
		char buff[len + 1];

		/*!
		 * FIXME: @see do_parse_nginx_log_item, I haven't found a better way yet
		 */
		memcpy(buff, item.first, len);	/*@note: NOT strcpy*/
		for(auto p = buff; p + 1 != buff + len; ++p){
			if(*p == '\0' && *(p + 1) == ' '){
				*p = '"';
			}
		}
		buff[len - 1] = '\n';
		std::replace(buff, buff + len, '\0', ' ');
		buff[len] = '\0';
		auto result = fwrite(buff, sizeof(char), len, f);

		if(result < (size_t)len || ferror(f))
			++n;
	}
}

int split_nginx_log(std::unordered_map<std::string, nginx_domain_stat> const& stats,
		char const * folder, char const * fmt)
{
	if(!folder || folder[0] == '\0' || !fmt || fmt[0] == '\0')
		return -1;

	size_t n = 0;
	std::map<std::string, FILE *> filemap; /*for output filenames*/
	for(auto const& dstat : stats){
		for(auto const& item : dstat.second._stats){
			auto site_id = dstat.second._site_id, user_id = dstat.second._user_id;
			char buft1[32], buft2[32];
			auto && fname = parse_nginx_split_filename(fmt,
					item.first.c_str_r(buft1, sizeof(buft1)), item.first.c_str_r(buft2, sizeof(buft2), "%Y%m%d"), site_id, user_id);
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
				continue;
			}

			fwrite_nginx_raw_log(file, item.second._logs, n);
		}
	}
	for (auto & it : filemap) {
		if (it.second)
			fclose(it.second);
	}
	if(n != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, n);
	return 0;
}
