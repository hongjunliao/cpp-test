/*!
 * This file is PART of nginx_log_analysis
 * rotate nginx log
 * @author hongjun.liao <docici@126.com>
 * @date 2017/02
 */
#include <cstdio>
#include <cstring>			/* memset */
#include <ctime>			/* time_t */
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 			/*std::string*/
#include <boost/filesystem.hpp>		/* boost::filesystem::file_size, recursive_directory_iterator, ... */
#include "test_options.h"		/*plcdn_la_options*/
#include "nginx_log_analysis.h"	/*nginx_domain_stat, ...*/

/*main.cpp*/
extern time_t g_plcdn_la_start_time;
extern std::unordered_map<std::string, site_info> g_sitelist;
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

struct rotate_file
{
	FILE * file;

};
static int parse_nginx_log_item_time(time_t & t);

/* remove files from dir @param rotate_dir if expired */
static void nginx_rotate_remove_expire(char const * rotate_dir, time_t now, int expire_sec);

/* append param row to dir @param rotate_dir, by time @param t, returns @param f for that file
 * @return 0 onccess */
static int nginx_rotate_append_log(char const * rotate_dir, char const * row, time_t const& t, FILE *& f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////

static void nginx_rotate_remove_expire(char const * rotate_dir, time_t now, int expire_sec)
{
	if(expire_sec <= 0)
		return;
	char buff[32] = "";
	tm t;
	strftime(buff, sizeof(buff), "%Y%m%d%H%M%S", localtime_r(&now, &t));
	auto id_2 = std::stoul(buff);

	std::vector<std::string> vec;
	for(boost::filesystem::directory_iterator it(rotate_dir), end; it != end; ++it){
		auto id_1 = std::stoul(it->path().filename().string()) * 100;	/* x100 for seconds, see nginx_rotate_append_log */
		if(id_2 - id_1 > (size_t)expire_sec)
			vec.push_back(it->path().string());
	}
	for(auto const& item : vec){
		boost::system::error_code ec;
		boost::filesystem::remove(item, ec);
	}
	return;
}

static int parse_nginx_log_item_time(time_t & t)
{
	return -1;
}

static int nginx_rotate_append_log(char const * rotate_dir, char const * row, time_t const& t, FILE *& f)
{
	time_group tg(t);
	char buft[32];
	tg.c_str_r(buft, sizeof(buft));

	boost::filesystem::path fullpath = rotate_dir;
	fullpath /= buft;
	f = fopen(fullpath.c_str(), "a");
	if(!f)
		return -1;
	auto result = fwrite(row, sizeof(char), strlen(row), f);
	if(result < strlen(row) || ferror(f))
		return -1;
	return 0;
}

/* append log from file @param logfile one by one to dir @param rotate_dir,
 * remove expired from @param rotate_dir, then analysis dir @param rotate_dir if modified
 * @param total_line, total line of file @param logfile
 * @notes:
 * 1.@param logfile can be NULL; if NULL, analysis dir @param rotate_dir only
 * 2.files in dir @param rotate_dir sample:
 *   /tmp/rotate_dir/201602140930, 201602140935, 201602140940, ...
 *   @see plcdn_la_options.interval
 * @return 0 on success */
int nginx_rotate_log(char const * rotate_dir, FILE * logfile, size_t& total_line,
		std::unordered_map<std::string, nginx_domain_stat> & logstats)
{
	if(!rotate_dir || rotate_dir[0] == '\0')
		return -1;
	boost::system::error_code ec;
	auto r = boost::filesystem::is_directory(rotate_dir, ec);
	if(!r || ec)
		return -1;
	/* clear expired */
	nginx_rotate_remove_expire(rotate_dir, g_plcdn_la_start_time, plcdn_la_opt.nginx_rotate_time);

	if(!logfile && plcdn_la_opt.verbose > 3){
		fprintf(stderr, "%s: logfile NULL, analysis rotate dir only\n", __FUNCTION__);
	}
	size_t failed_line = 0;	/* total_lines for parse failed */

	/* append log by time first */
	total_line = 0;
	std::map<time_group, rotate_file> rmap;
    char buf[1024 * 10];	/* length of 1 row */
    while (fgets(buf, sizeof(buf), logfile)){
    	++total_line;
    	time_t t;
    	int result = parse_nginx_log_item_time(t);
    	if(result != 0){
    		if(plcdn_la_opt.verbose > 4)
    			fprintf(stderr, "%s: parse_nginx_log_item_time failed, line=%zu, skip\n", __FUNCTION__, total_line);
    		++failed_line;
    		continue;
    	}
    	FILE * file = NULL;
		result = nginx_rotate_append_log(rotate_dir, buf, t, file);
		if(result == 0 && file)
			rmap[t].file = file;
    }
    /* parse rotate_dir(updated only) and do statistics */
    for(auto const& item: rmap){
    	auto is_time_in = is_time_in_range(item.first.t(), plcdn_la_opt.begin_time, plcdn_la_opt.end_time);
    	if(!is_time_in)
    		continue;
    	auto f = item.second.file;
    	f = std::freopen(NULL, "r", f);
		if(!f){
			if(plcdn_la_opt.verbose > 4){
				char buft[32] = "<error>";
				item.first.c_str_r(buft, sizeof(buft));
				fprintf(stderr, "%s: freopen failed, skip! rotate_dir = '%s', file = '%s'\n", __FUNCTION__,
						rotate_dir, buft);
			}
			continue;
		}
		size_t n = 0;
		do_nginx_log_stats(f, plcdn_la_opt, g_sitelist, logstats, n);
    }
	return 0;
}
