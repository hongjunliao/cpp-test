/*!
 * This file is PART of plcdn_log_analysis
 * parse logs and print results, currently support: nginx, srs logfiles
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 * @NOTES:
 * 6.merge srs results  to nginx results currently, see append_flow_nginx, @date 2016/11/14
 *
 * compile and link notes:
 * 1.gcc define ENABLE_IPMAP to enable libipmap
 * 2.gcc enable c++11: -std=c++0x
 * 3.gcc include path add -I../inc/ or -I"${workspace_loc:/${ProjName}/inc}"
 * 4.gcc add -fPIC for shared libraries on *nix
 * 5.gcc add -lboost_regex -lboost_filesystem  -lboost_system -lcrypto -lrt -pthread -lpopt,
 *   -lipmap if defined ENABLE_IPMAP
 * 7.define _GNU_SOURCE on cygwin!!!
 *
 */

#include <stdio.h>
#include "bd_test.h"			/*test_srs_log_stats_main*/
#include "test_options.h" 		/*sla_options*/

#include <stdio.h>
#include <string.h> 	/*strncpy*/
#include <sys/sysinfo.h>	/*get_nprocs*/
#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
//#include <locale.h> 	/*setlocale*/
#include <pthread.h> 	/*pthread_create*/
#include <unordered_map> 	/*std::unordered_map*/
#include <map>				/*std::map*/
//#include <thread>		/*std::thread*/
//#include <atomic>		/*std::atomic*/
#include "bd_test.h"		/*test_nginx_log_stats_main*/
#include "test_options.h"	/*nla_options**/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "srs_log_analysis.h"	/*srs_log_context*/
#include "string_util.h"	/*md5sum*/
#include "net_util.h"	/*get_if_addrs, ...*/
#include <algorithm>	/*std::min*/
#include <boost/filesystem.hpp> /*create_directories*/
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*for nginx log*/

/*tests, @see nginx_log_analysis/test.cpp*/
extern int test_nginx_log_analysis_main(int argc, char ** argv);

/*split_log.cpp*/
extern int split_nginx_log(std::unordered_map<std::string, nginx_domain_stat> const& stats,
		char const * folder, char const * fmt);

/*parse nginx_log buffer @apram ct, and output results*/
static int parse_nginx_log_item_buf(parse_context& ct);
/* split file @param f into parts, use pthread to parallel parse*/
static int parallel_parse_nginx_log(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<std::string, nginx_domain_stat> & stats);

/*load devicelist, @param devicelist map<ip, device_id>*/
static int load_devicelist(char const* file, std::unordered_map<std::string, int>& devicelist);

/*get device_id by ip*/
static int get_device_id(std::unordered_map<std::string, int> const& devicelist);

/* url manage:
 * if @param str NOT exist, then add as new, else return exist
 * this funtions currently NOT used because of poor speedup, @date: 2016/10/27
 */
static char const * str_find(char const *str, int len = -1);

/*plcdn_log_analysis/print_table.cpp*/
extern int print_nginx_log_stats(std::unordered_map<std::string, nginx_domain_stat> const& logstats);
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*for srs log*/
static int extract_and_parse_srs_log(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<std::string, srs_domain_stat> & logstats);
/*print_table.cpp*/
extern void fprint_srs_log_stats(std::unordered_map<std::string, srs_domain_stat> const& srs_stats);
/*split_log.cpp*/
extern void split_srs_log_by_sid(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<int, srs_sid_log> & slogs);
extern void sync_srs_sids_dir(std::unordered_map<int, srs_sid_log> & slogs,
		char const * srs_sid_dir);
extern int parse_srs_log(std::unordered_map<int, srs_sid_log> & slogs,
		std::unordered_map<std::string, srs_domain_stat> & logstats);
extern int split_srs_log(std::unordered_map<std::string, srs_domain_stat> const & logstats,
		char const * folder, char const * fmt);
extern int fwrite_srs_log_by_sid(std::unordered_map<int, srs_sid_log> & slogs, char const * folder);

/* plcdn_log_result_merge/main.cpp */
extern int merge_srs_flow_user(int argc, char ** argv);

/* nginx_rotate.cpp */
extern int nginx_rotate_log(char const * rotate_dir, int rotate_time, FILE * logfile,
		size_t& total_line, size_t & failed_line,
		std::unordered_map<std::string, nginx_domain_stat> & logstats);

/* plcdn_log_transform.cpp */
extern int nginx_transform_log(FILE * in, FILE * out, int fmt);
/////////////////////////////////////////////////////////////////////////////////////////////////////
/*GLOBAL vars*/
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
/*map<ip_addr : device_id>*/
static std::unordered_map<std::string, int> g_devicelist;
/*map<domain, site_info>*/
std::unordered_map<std::string, site_info> g_sitelist;
static size_t g_nginx_total_line = 0, g_nginx_failed_line = 0;
/*srs_log_analysis/split_log.cpp*/
extern size_t g_srs_total_line;
extern size_t g_srs_failed_line;
extern size_t g_srs_slog_line;
extern size_t g_srs_trans_line;

time_t g_plcdn_la_start_time = 0;
int g_plcdn_la_device_id = 0;

///////////////////////////////////////////////////////////////////////////////////////////////

int load_devicelist(char const* file, std::unordered_map<std::string, int>& devicelist)
{
	if(!file) return -1;
	FILE * f = fopen(file, "r");
	if(!f) {
		fprintf(stderr, "%s: fopen file '%s' failed\n", __FUNCTION__, file);
		return 1;
	}
	char data[1024] = "";
	while(fgets(data, sizeof(data), f)){
		if(data[0] == '\n') continue;
		data[strlen(data) - 1] = '\0';
		char const * token = strtok(data, " ");
		int id = atoi(token);
		token = strtok(NULL, " ");
		devicelist[token] = id;
	}
//	for(auto const& item : devicelist){
//		fprintf(stdout, "[%s]--[%d]\n", item.first.c_str(), item.second);
//	}
	return 0;
}

static int get_device_id(std::unordered_map<std::string, int> const & devicelist)
{
	char ips[64][16];
	int count = 64;
	int result  = get_if_addrs(ips[0], count, 16);
	if(result != 0)
		return -1;
	//std::find_first_of();
	for(int i = 0; i < count; ++i){
		for(auto const& item : devicelist){
			if(item.first.compare(ips[i])  == 0)
				return item.second;
		}
	}
	return 0;
}

/*for str_find()*/
static int str_find_realloc(char *& p, size_t & total, size_t step_len)
{
	for(size_t i = 1; i < step_len; i *= 2){
		size_t new_sz = step_len / i;
		void * p2  = realloc(p, total + new_sz);
		if(p2) {
			p = (char * )p2;
			total += new_sz;
			if(plcdn_la_opt.verbose)
				fprintf(stdout, "%s: total=%s\n", __FUNCTION__, byte_to_mb_kb_str(total, "%-.0f %cB"));
			return 0;
		}
	}
	return -1;
}

static char const * str_find(char const *str, int len)
{
	if(!str || !str[0])
		return NULL;
	/*FIXME: dangerous in multi-threaded!!! use __thread/thread_local?*/
	static /*thread_local*/ std::unordered_map<std::string, char *> urls;
	static size_t step = 10 * 1024, total = 1024 * 64;	/*KB*/
	auto start_p = (char *)malloc(total);
	static size_t offset_len = 0;
	if(len == -2){
		free(start_p);
		return 0;
	}
	len = (len != -1? len : strlen(str));
	char md5buff[33];
	md5sum_r(str, len, md5buff);
	auto &s = urls[md5buff];
	if(!s){
		if(offset_len + len + 1> total){
			if(str_find_realloc(start_p, total, step) != 0)
				return NULL;
		}
		s = start_p + offset_len;
		strncpy(s, str, len);
		s[len] = '\0';
		offset_len += (len + 1);
	}
	return s;
}

int parse_nginx_log_item_buf(parse_context& ct)
{
	auto & buf = ct.buf;
	auto & len = ct.len;
	auto & logstats = ct.logstats;
	auto & total_lines = ct.total_lines = 0;
	auto & failed_lines = ct.failed_lines = 0;

	log_item item;
	for(char * p = buf; p != buf + len; ++p){
//		printf(">_____");
//		for(auto p1 = buf; p1 != buf + len; ++p1){
//			printf("%c", *p1);
//		}
//		printf("_____|\n");

		int result = parse_log_item(item, p, '\n');

		if(result == 0){
			auto is_time_in = is_time_in_range(item.time_local, plcdn_la_opt.begin_time, plcdn_la_opt.end_time);
			if(is_time_in)
				do_nginx_log_stats(item, plcdn_la_opt, g_sitelist, logstats);
		}
		else {
			++failed_lines;
			//current line failed, move to next line
			while(p != buf + len && !(*p == '\0' || *p == '\n')) { ++p; }
		}
		++total_lines;
	}
	return 0;
}

static void * parallel_parse_thread_func(void * varg)
{
	if(!varg) return varg;
	auto & arg = *(parse_context*)varg;
	/*arg->buf, arg->len, arg->logstats, arg->total_lines, arg->failed_lines*/
	parse_nginx_log_item_buf(arg);

	return varg;
}

/*FIXME: nginx_domain_stat._logs*/
static int log_stats_append(std::unordered_map<std::string, nginx_domain_stat> & a,
		std::unordered_map<std::string, nginx_domain_stat> const& b)
{
	for(auto const& item : b){
		for(auto const &stat : item.second._stats)
			a[item.first]._stats[stat.first] += stat.second;
	}
	return 0;
}

static int parallel_parse_nginx_log(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<std::string, nginx_domain_stat> & stats)
{
	int parallel_count = get_nprocs() - 1;			/*parallel count, exclude main_thread, maybe 0*/
	size_t min_bytes = 1024 * 1024 * 64; 	/*min 64MB*/
	for(size_t c = logfile_stat.st_size / min_bytes; c < (size_t)parallel_count; --parallel_count){ /*empty*/ }
	/*!
	 * FIXME:
	 * 1. be aware of local static vars when in multi-thread
	 * 2. disabled by default, because of little speedup
	 */
	if(!plcdn_la_opt.enable_multi_thread)
		parallel_count = 0;
	if(plcdn_la_opt.verbose)
		fprintf(stdout, "%s: logfile_size = %zd/%s, para_count=%d\n", __FUNCTION__
				, logfile_stat.st_size, byte_to_mb_kb_str(logfile_stat.st_size, "%-.2f %cB"), parallel_count);

	pthread_t threads[parallel_count];
	parse_context parse_args[parallel_count + 1];	/*include main_thread*/
	off_t offset_p = 0;
	for(int i = 0; i < parallel_count; ++i){
		static size_t len = logfile_stat.st_size / (parallel_count + 1);
		size_t nlen = (strchr(start_p  + offset_p + len - 1024 * 8, '\n') + 1 - start_p) - offset_p;

		auto& arg = parse_args[i];
		arg.buf = (char *)start_p + offset_p;
		arg.len = nlen;
		arg.total_lines = 0;
		pthread_t tid;
		int result = pthread_create(&tid, NULL, parallel_parse_thread_func, &arg);
		if(result != 0){
			fprintf(stderr, "%s: pthread_create() failed, parallel index=%d\n", __FUNCTION__, i);
			return 1;
		}
		if(plcdn_la_opt.verbose){
			char buff[64];
			printf("%s: worker_thread=%p, process size=%zu/%s, percent=%.1f%%\n", __FUNCTION__,
					&tid, nlen, byte_to_mb_kb_str_r(nlen, "%-.2f %cB", buff), (double)nlen * 100/ logfile_stat.st_size);
		}

		threads[i] = tid;
		offset_p += nlen;
	}
	size_t left_len = logfile_stat.st_size - offset_p;
	if(plcdn_la_opt.verbose){
		char buff[64];
		printf("%s: main_thread process size=%zu/%s, percent=%.1f%%\n", __FUNCTION__,
				left_len, byte_to_mb_kb_str_r(left_len, "%-.2f %cB", buff), (double)left_len * 100/ logfile_stat.st_size);
	}
	auto & arg = parse_args[parallel_count];
	arg.buf = start_p + offset_p;
	arg.len = left_len;
	arg.total_lines = 0;
	parse_nginx_log_item_buf(arg);

	for(auto & item : threads){
		void * tret;
		pthread_join(item, &tret);
		if(plcdn_la_opt.verbose)
			fprintf(stdout, "%s: thread=%p exited\n", __FUNCTION__, &item);
		auto & ct = *(parse_context * )tret;
		log_stats_append(stats, ct.logstats);
		g_nginx_total_line += ct.total_lines;
		g_nginx_failed_line += ct.failed_lines;
	}
	if(parallel_count == 0){
		/*FIXME: NOT needed? just for speed*/
		stats = parse_args[parallel_count].logstats;
	}
	else{
		log_stats_append(stats, parse_args[parallel_count].logstats);
	}
	g_nginx_total_line += parse_args[parallel_count].total_lines;
	g_nginx_failed_line += parse_args[parallel_count].failed_lines;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//for srs log analysis

/*extract useful log items and parse*/
int extract_and_parse_srs_log(char * start_p, struct stat const & logfile_stat,
		std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	size_t linecount = 0, failed_count = 0, skip_count = 0;
	std::vector<srs_connect_ip> ip_items;
	std::vector<srs_connect_url> url_items;

	/*FIXME empty line!!*/
	srs_log_item logitem;
	for(char * p = start_p, * q = p; q != start_p + logfile_stat.st_size; ++q){
		if(*q == '\n'){
			*q = '\0';
			++linecount;

			int log_type;
			/*FIXME: srs_connect_ip and srs_connect_url always comes before srs_trans?*/
			auto status = parse_srs_log_item(p, logitem, log_type);
			if(status == 0) {
				switch(log_type){
				case 2:
					status = do_srs_log_stats(logitem, log_type, ip_items, url_items, logstats);
					if(status != 0)
						++failed_count;
					break;
				case 1:
					ip_items.push_back(logitem.conn_ip);
					break;
				case 4:
					url_items.push_back(logitem.conn_url);
					break;
				case 0:
					++skip_count;
					break;
				default:
					break;
				}
			}
			else
				++failed_count;
			p = q + 1;
		}
	}
	if(plcdn_la_opt.verbose)
		fprintf(stdout, "%s: processed, total_line: %zu, failed=%zu, skip=%zu\n", __FUNCTION__
				, linecount, failed_count, skip_count);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*merge*/

static void append_flow_nginx(
		std::unordered_map<std::string, nginx_domain_stat> & nginx_stats,
		std::unordered_map<std::string, srs_domain_stat> const& srs_stats)
{
	if(plcdn_la_opt.verbose)
		fprintf(stdout, "%s: appending srs flow to nginx ...\n", __FUNCTION__);
	for(auto srs_domain_pair : srs_stats){
		auto & dstat = nginx_stats[srs_domain_pair.first];
		if(dstat._site_id == 0 || dstat._user_id == 0){
			dstat._site_id = srs_domain_pair.second._site_id;
			dstat._user_id = srs_domain_pair.second._user_id;
		}
		else{
			if(dstat._site_id != srs_domain_pair.second._site_id ||
					dstat._user_id != srs_domain_pair.second._user_id){
				fprintf(stderr, "%s: interval error! site_id or user_id not equal in nginx and srs log! domain='%s'\n",
						__FUNCTION__, srs_domain_pair.first.c_str());
				 continue;
			}
		}
		for(auto & srs_log_pair : srs_domain_pair.second._stats){
			auto & nginx_stat = dstat._stats[srs_log_pair.first];
			auto & srs_stat = srs_log_pair.second;

			nginx_stat.srs_in = srs_stat.ibytes_total();
			nginx_stat.srs_out = srs_stat.obytes_total();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//main
int test_plcdn_log_analysis_main(int argc, char ** argv)
{
//	test_nginx_log_analysis_main(argc, argv);	/*for test only*/

	int result = plcdn_la_parse_options(argc, argv);
	if(result != 0 || plcdn_la_opt.show_help){
		plcdn_la_opt.show_help? plcdn_la_show_help(stdout) :
				plcdn_la_show_usage(stdout);
		return 0;
	}
	if(plcdn_la_opt.show_version){
		fprintf(stdout,"build at %s %s\n", __DATE__, __TIME__);
		return 0;
	};
	if(plcdn_la_opt.verbose)
		plcdn_la_options_fprint(stdout, &plcdn_la_opt);
	if(plcdn_la_opt.print_device_id){	//query device_id and return
		int result = load_devicelist(plcdn_la_opt.devicelist_file, g_devicelist);
		int id = result == 0? get_device_id(g_devicelist) : 0;
		fprintf(stdout, "%d\n", id);
		return result == 0? 0 : 1;
	}
	if(plcdn_la_opt.nginx_trans_log)
		return nginx_transform_log(stdin, stdout, plcdn_la_opt.nginx_trans_log);
	if(plcdn_la_opt.work_mode == 1)
		return merge_srs_flow_user(argc, argv);
	g_plcdn_la_start_time = time(NULL);
	result = load_devicelist(plcdn_la_opt.devicelist_file, g_devicelist);
	if(result != 0){
		fprintf(stderr, "%s: load_devicelist() failed\n", __FUNCTION__);
		return 1;
	}
	g_plcdn_la_device_id = plcdn_la_opt.device_id > 0? plcdn_la_opt.device_id : get_device_id(g_devicelist);

	result = load_sitelist(plcdn_la_opt.siteuidlist_file, g_sitelist);
	if(result != 0){
		fprintf(stderr, "%s: load_sitelist() failed\n", __FUNCTION__);
		return 1;
	}
#ifdef ENABLE_IPMAP
	if(plcdn_la_opt.output_file_ip_source && 0 != locisp_group::load_ipmap_file(plcdn_la_opt.ipmap_file))  {
		fprintf(stderr, "%s: ipmap_load(%s) failed\n", __FUNCTION__, plcdn_la_opt.ipmap_file);
		return 1;
	}
#else
	fprintf(stdout, "%s: ipmap DISABLED on this platform\n", __FUNCTION__);
#endif //ENABLE_IPMAP
	int interval = plcdn_la_opt.interval;
	if(plcdn_la_opt.verbose && (interval < 300 || interval > 3600)){
		fprintf(stdout, "%s: WARNING, interval(%d) too %s\n", __FUNCTION__, interval, interval < 300? "small" : "large");
	}
	time_group::_sec = interval;

	/*parse logs*/

	/*inputs*/
	FILE * nginx_log_file = NULL, * srs_log_file = NULL;
	char * nginx_file_addr = NULL, * srs_file_addr = NULL ;
	struct stat nginx_file_stat, srs_file_stat;
	/*outputs*/
	std::unordered_map<std::string, nginx_domain_stat> nginx_logstats;	/*[domain: domain_stat]*/
	std::unordered_map<std::string, srs_domain_stat>  srs_logstats;		/*[domain: srs_domain_stat]*/

	/*for nginx_log*/
	if(plcdn_la_opt.nginx_log_file){
		nginx_log_file = fopen(plcdn_la_opt.nginx_log_file, "r");
		if(!nginx_log_file) {
			fprintf(stderr, "%s: fopen file '%s' failed\n", __FUNCTION__, plcdn_la_opt.nginx_log_file);
			return 1;
		}

		if(plcdn_la_opt.work_mode == 2)	{ /* rotate mode */
			auto result = nginx_rotate_log(plcdn_la_opt.nginx_rotate_dir, plcdn_la_opt.nginx_rotate_time,
							nginx_log_file, g_nginx_total_line, g_nginx_failed_line, nginx_logstats);
			if(result != 0 && plcdn_la_opt.verbose)
				fprintf(stderr, "%s: nginx_rotate_log failed, re-run required\n", __FUNCTION__);
		}
		else{	/* analysis mode */
			auto fno = fileno(nginx_log_file);
			if(fstat(fno, &nginx_file_stat) < 0){
				fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "nginx_log_file");
				return 1;
			}
			/*FIXME: PAGE_SIZE?*/
			nginx_file_addr = (char *)mmap(NULL, nginx_file_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fno, 0);
			if(!nginx_file_addr || nginx_file_addr == MAP_FAILED){
				fprintf(stderr, "%s: mmap() failed for %s\n", __FUNCTION__, "nginx_log_file");
				return 1;
			}

			auto status = parallel_parse_nginx_log(nginx_file_addr, nginx_file_stat, nginx_logstats);
			if(status != 0){
				fprintf(stderr, "%s: parallel_parse_nginx_log failed, exit\n", __FUNCTION__);
				return 1;
			}
		}
		if(plcdn_la_opt.verbose){
			auto color = g_nginx_failed_line > 0? 31 : 0;
			fprintf(stdout, "%s: processed nginx log '%s', total=%zu, failed=%zu, \e[%dm%.1f%%\e[0m failed\n", __FUNCTION__,
					plcdn_la_opt.nginx_log_file, g_nginx_total_line, g_nginx_failed_line, color, g_nginx_failed_line * 100.0 / g_nginx_total_line);
		}
		/*split log*/
		if(plcdn_la_opt.output_split_nginx_log){
			if(plcdn_la_opt.verbose)
				fprintf(stdout, "%s: splitting nginx log file: '%s'...\n", __FUNCTION__, plcdn_la_opt.nginx_log_file);
			auto status = split_nginx_log(nginx_logstats,
					plcdn_la_opt.output_split_nginx_log, plcdn_la_opt.format_split_nginx_log);
		}
	}

	/*for srs_log*/
	if(plcdn_la_opt.srs_log_file){
		srs_log_file = fopen(plcdn_la_opt.srs_log_file, "r");
		if(!srs_log_file) {
			fprintf(stderr, "%s: fopen file '%s' failed\n", __FUNCTION__, plcdn_la_opt.srs_log_file);
			return 1;
		}
		auto fno = fileno(srs_log_file);
		if(fstat(fno, &srs_file_stat) < 0){
			fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "srs_log_file");
			return 1;
		}
		/*FIXME: PAGE_SIZE?*/
		srs_file_addr = (char *)mmap(NULL, srs_file_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fno, 0);
		if(!srs_file_addr || srs_file_addr == MAP_FAILED){
			fprintf(stderr, "%s: mmap() failed for %s\n", __FUNCTION__, "srs_log_file");
			return 1;
		}

		/*FIXME: the return value is wired, we NOT use it yet*/
		boost::system::error_code ec;
		boost::filesystem::create_directories(plcdn_la_opt.srs_sid_dir, ec);

		std::unordered_map<int, srs_sid_log> slogs;
		split_srs_log_by_sid(srs_file_addr, srs_file_stat, slogs);
		sync_srs_sids_dir(slogs, plcdn_la_opt.srs_sid_dir);
		if(plcdn_la_opt.output_split_srs_log_by_sid){
			boost::filesystem::create_directories(plcdn_la_opt.output_split_srs_log_by_sid, ec);
			fwrite_srs_log_by_sid(slogs, plcdn_la_opt.output_split_srs_log_by_sid);
		}
		auto status = parse_srs_log(slogs, srs_logstats);
		if(status != 0){
			fprintf(stderr, "%s: parse_srs_log failed, exit\n", __FUNCTION__);
			return 1;
		}
		if(plcdn_la_opt.verbose){
			fprintf(stdout, "%s: processed srs log '%s', total_line = %zu, failed = %zu, trans_log = %zu\n", __FUNCTION__,
					plcdn_la_opt.srs_log_file,
					g_srs_total_line, g_srs_failed_line, g_srs_trans_line);
		}
		if(plcdn_la_opt.output_split_srs_log){
			if(plcdn_la_opt.verbose)
				fprintf(stdout, "%s: splitting srs log file '%s'...\n", __FUNCTION__, plcdn_la_opt.srs_log_file);
			auto status = split_srs_log(srs_logstats,
					plcdn_la_opt.output_split_srs_log, plcdn_la_opt.format_split_srs_log);
		}
		fprint_srs_log_stats(srs_logstats);
	}
	if(!nginx_log_file && !srs_log_file){
		fprintf(stderr,  "%s: none of nginx, srs log file specified or can be read\n", __FUNCTION__);
		return 1;
	}
	if(plcdn_la_opt.append_flow_nginx)
		append_flow_nginx(nginx_logstats, srs_logstats);

	/*output results*/
	print_nginx_log_stats(nginx_logstats);

	/*uinit*/
	if(nginx_file_addr)
		munmap(nginx_file_addr, nginx_file_stat.st_size);
	if(srs_file_addr)
		munmap(srs_file_addr, srs_file_stat.st_size);
	return 0;
}
