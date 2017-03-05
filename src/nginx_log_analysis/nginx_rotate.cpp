/*!
 * This file is PART of nginx_log_analysis
 * rotate nginx log
 * @author hongjun.liao <docici@126.com>
 * @date 2017/02
 */
#include <cstdio>
#include <cstring>			/* memset */
#include <ctime>			/* time_t, strptime */
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 			/*std::string*/
#include <boost/regex.hpp> 		/*boost::regex_match*/
#include <boost/filesystem.hpp>		/* boost::filesystem::file_size, recursive_directory_iterator, ... */
#include "test_options.h"		/*plcdn_la_options*/
#include "nginx_log_analysis.h"	/*nginx_domain_stat, ...*/

/*main.cpp*/
extern std::unordered_map<std::string, site_info> g_sitelist;
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

/* this var is valid only when plcdn_la_options.work_mode == 2, @see nginx_rotate_log */
time_t g_nginx_rotate_time = 0;

struct rotate_file
{
	FILE * file;
};

/* convert format "%Y%m%d%H%M" to time_t, return 0 if failed */
static time_t s_to_time_t(std::string const& s);
/* get last rotate time, if not exists set @param t to 0 */
static void nginx_rotate_get_lasttime(char const * rotate_dir, time_t & t);

static int parse_nginx_log_item_time_and_domain(char const * buff, time_t & t, std::string & domain);

/* remove files from dir @param rotate_dir if expired */
static void nginx_rotate_remove_expire(char const * rotate_dir, time_t now, int expire_sec);

/* append param row to dir @param rotate_dir, by time @param t, returns @param f for that file
 * @return 0 onccess */
static int nginx_rotate_append_log(char const * rotate_dir, char const * row, time_group const& tg, FILE *& f);

/* save @param now to dir @param rotate_dir/.last_rotate_time
 * @see nginx_rotate_get_lasttime,
 * TODO: impliment it */
static void nginx_rotate_save_last_rotate_time(char const * rotate_dir, time_t now);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

static time_t s_to_time_t(std::string const& s)
{
	tm my_tm{ 0 };
	char const * result = strptime(s.c_str(), "%Y%m%d%H%M" , &my_tm);
	if(!result)
		return 0;
	my_tm.tm_isdst = 0;
	auto ret = mktime(&my_tm);
	return ret;
}

static void nginx_rotate_remove_expire(char const * rotate_dir, time_t now, int expire_sec)
{
	if(expire_sec <= 0)
		return;
	std::vector<std::string> vec;
	for(boost::filesystem::directory_iterator it(rotate_dir), end; it != end; ++it){
		auto fname = it->path().filename().string();
		static boost::regex const re("[0-9]{12}");
		if(!boost::filesystem::is_regular_file(*it) || !boost::regex_match(fname, re))
			continue;
		auto t = s_to_time_t(fname);
		if(std::difftime(now, t) > (size_t)expire_sec)
			vec.push_back(it->path().string());
	}
	if(!vec.empty() && plcdn_la_opt.verbose > 2){
		fprintf(stdout, "%s: in '%s', expired:\n", __FUNCTION__, rotate_dir);
		for(auto const & item : vec)
			fprintf(stdout, "\t%s\n", item.c_str());
	}
	for(auto const& item : vec){
		boost::system::error_code ec;
		auto r = boost::filesystem::remove(item, ec);
		if((!r || ec) && plcdn_la_opt.verbose > 4)
			fprintf(stderr, "%s: WARNING, remove file '%s' failed\n", __FUNCTION__, item.c_str());
	}
	return;
}

static int parse_nginx_log_item_time_and_domain(char const * buff, time_t & t, std::string & domain)
{
	std::pair<char const *, char const *> items[18];
	int result = do_parse_nginx_log_item(items, buff, '\n');
	if(result != 0)
		return -1;
	domain.assign(items[0].first, items[0].second);

	/* @see parse_nginx_log_item */
	auto len = items[4].second - items[4].first;
	char item4[len + 1];
	strncpy(item4, items[4].first, len);
	item4[len] = '\0';

	tm my_tm;
	if(!strptime(item4 + 1, "%d/%b/%Y:%H:%M:%S" , &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	t = mktime(&my_tm);

	return 0;
}

static int nginx_rotate_append_log(char const * rotate_dir, char const * row, time_group const& tg, FILE *& f)
{
	char buft[32] = ".error";
	tg.c_str_r(buft, sizeof(buft));

	boost::filesystem::path fullpath = rotate_dir;
	fullpath /= buft;
	if(!f){
		f = fopen(fullpath.c_str(), "a");
		if(!f)
			return -1;
	}
	auto result = fwrite(row, sizeof(char), strlen(row), f);
	if(result < strlen(row) || ferror(f))
		return -1;
	return 0;
}

static void nginx_rotate_get_lasttime(char const * rotate_dir, time_t & t)
{
	t = 0;
	for(boost::filesystem::directory_iterator it(rotate_dir), end; it != end; ++it){
		auto fname = it->path().filename().string();
		static boost::regex const re("[0-9]{12}");
		if(!boost::filesystem::is_regular_file(*it) || !boost::regex_match(fname, re))
			continue;
		auto t2 = s_to_time_t(fname);
		if(t2 == 0)
			continue;
		if(t2 > t)
			t = t2;
	}
//	fprintf(stdout, "%s: time = %ld\n", __FUNCTION__, t);
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
int nginx_rotate_log(char const * rotate_dir, int rotate_time, FILE * logfile,
		size_t& total_line, size_t & failed_line,
		std::unordered_map<std::string, nginx_domain_stat> & logstats)
{
	if(!rotate_dir || rotate_dir[0] == '\0')
		return -1;
	boost::system::error_code ec;
	auto r = boost::filesystem::create_directory(rotate_dir, ec);
	if(ec){
		if(!logfile && plcdn_la_opt.verbose > 3)
			fprintf(stderr, "%s: create_directory '%s' failed\n", __FUNCTION__, rotate_dir);
		return -1;
	}
	r = boost::filesystem::is_directory(rotate_dir, ec);
	if(!r || ec)
		return -1;

	if(!logfile && plcdn_la_opt.verbose > 3){
		fprintf(stderr, "%s: logfile NULL, analysis rotate dir only\n", __FUNCTION__);
	}

	if(plcdn_la_opt.verbose > 4)
		fprintf(stdout, "%s: rotating ...\n", __FUNCTION__);
    time_t lastt = 0;	/* last time*/
	nginx_rotate_get_lasttime(rotate_dir, lastt);

	/* append log by time first */
    size_t skipped_line = 0;
    std::vector<size_t> failednums;
	std::map<time_group, rotate_file> rmap;
    char buf[1024 * 10];	/* max length of 1 row */
    while (fgets(buf, sizeof(buf), logfile)){
    	++total_line;
    	time_t t;
    	std::string domain;
    	int result = parse_nginx_log_item_time_and_domain(buf, t, domain);
    	if(result != 0){
    		if(plcdn_la_opt.verbose > 4)
    			failednums.push_back(total_line);
    		++failed_line;
    		continue;
    	}
    	if(t > lastt)
    		lastt  = t;
    	/* try to skip too old log, according to rotate_dir */
    	if(lastt != 0){
			auto difft = std::difftime(lastt, t);
			if(difft > (double)rotate_time){
				if(plcdn_la_opt.verbose > 3)
					fprintf(stdout, "%s: WARNING!!! time expired, skipped. log:\n%s\n", __FUNCTION__, buf);
				++skipped_line;
				continue;	/* time too old, this row need to be skipped */
			}
    	}
    	time_group tg(t);
    	/* @NOTES: this is the ONLY place where we save the new comming logs, which is used in @see split_nginx_log
    	 * and we use std::string, NOT std::pair
    	 * TODO: compress buf? */
    	/* FIXME: tool old log still be added if rodate_dir empty, added temporary */
    	logstats[domain]._stats[tg]._logs.push_back(buf);

		result = nginx_rotate_append_log(rotate_dir, buf, tg, rmap[tg].file);
    }
    if(!failednums.empty() && plcdn_la_opt.verbose > 4){
		fprintf(stdout, "%s: parse_nginx_log_item_time failed lines: [", __FUNCTION__);
		for(auto const & item : failednums)
			fprintf(stdout, "%zu,", item);
		fprintf(stdout, "]\n");
    }
    g_nginx_rotate_time = lastt;
//    nginx_rotate_save_last_rotate_time(rotate_dir, lastt);
	if(plcdn_la_opt.verbose > 4)
		fprintf(stdout, "%s: rotate done, total = %zu, skipped = %zu\n", __FUNCTION__, total_line, skipped_line);
	/* clear expired */
	nginx_rotate_remove_expire(rotate_dir, lastt, rotate_time);
    /* parse rotate_dir(updated only) and do statistics */
	if(!rmap.empty() && plcdn_la_opt.verbose > 4){
		fprintf(stdout, "%s: '%s', need re-statistics: [", __FUNCTION__, rotate_dir);
		for(auto const & item : rmap){
			char buft[32] = "<error>";
			item.first.c_str_r(buft, sizeof(buft));
			fprintf(stdout, "%s, ", buft);
		}
		fprintf(stdout, "]\n");
	}
	if(plcdn_la_opt.verbose > 4){
		fprintf(stdout, "%s: statistics ...\n", __FUNCTION__);
	}
    for(auto & item: rmap){
    	auto f = item.second.file;
    	if(!f) continue;
		char buft[32] = "<error>";
		item.first.c_str_r(buft, sizeof(buft));
		/* try to skip too old log, according to rotate_dir */
    	boost::system::error_code ec;
    	boost::filesystem::path fpath(rotate_dir);
    	fpath /= buft;
    	auto r = boost::filesystem::exists(fpath, ec);
    	if(ec || !r){
    		fclose(f);
			if(plcdn_la_opt.verbose > 3)
				fprintf(stdout, "%s: remove expired rotate file, time='%s'.\n", __FUNCTION__, buft);
    		continue;
    	}

    	auto is_time_in = is_time_in_range(item.first.t(), plcdn_la_opt.begin_time, plcdn_la_opt.end_time);
    	if(!is_time_in)
    		continue;
    	f = std::freopen(NULL, "r", f);
		if(!f){
			if(plcdn_la_opt.verbose > 4){
				fprintf(stderr, "%s: freopen failed, skip! rotate_dir = '%s', file = '%s'\n", __FUNCTION__,
						rotate_dir, buft);
			}
			continue;
		}
		size_t n = 0;
		do_nginx_log_stats(f, plcdn_la_opt, g_sitelist, logstats, n);
		fclose(f);
    }
	return 0;
}

