/*!
 * This file is PART of nginx_log_analysis
 * parse nginx logs and print results
 * @author hongjun.liao <docici@126.com>
 * @date 2016/9
 * @note:
 * 1.As of current design, the input nginx log file MUST have ONLY one domain, @see test_nginx_log_split_main
 * 2.the core design is to grouping logs into std::map/std::unordered_map by different conditions \
 * such as time_interval, locisp, ip, just like "group by" syntax in SQL, @see class log_stat
 *
 * @note:
 * 1.gcc define ENABLE_IPMAP to enable libipmap
 * 2.gcc enable c++11: -std=c++0x
 * 3.gcc include path add -I../inc/ or -I"${workspace_loc:/${ProjName}/inc}"
 * 4.gcc add -fPIC for shared libraries
 * 5.gcc add -lpopt -lpthread -lcrypto -lrt
 */
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
#include "string_util.h"	/*md5sum*/
#include "net_util.h"	/*get_if_addrs, ...*/
#include <algorithm>	/*std::min*/

/*tests, @see nginx_log_analysis/test.cpp*/
extern int test_nginx_log_analysis_main(int argc, char ** argv);
/*parse nginx_log buffer @apram ct, and output results*/
static int parse_nginx_log_item_buf(parse_context& ct);
/* split file @param f into parts, use pthread to parallel parse*/
static int parallel_parse_nginx_log(FILE * f, std::unordered_map<std::string, nginx_domain_stat> & stats);

/* parse nginx_log $request_uri field, return url, @param cache_status: MISS/MISS0/HIT,...
 * @param mode:
 * mode 0, url endwith ' ', reserve all, e.g. "POST /zzz.asp;.jpg HTTP/1.1", return "/zzz.asp;.jpg"
 * mode 1, url endwith '?'(if no '?', then endwith ' '), ignore parameters, e.g. "GET /V3/?page_id=1004&cid=1443 HTTP/1.1", return "/V3/"
 * mode 2, custom, @param cache_status required
 * */
static char * parse_nginx_log_request_uri_url(char * request_uri, int * len, char const * cache_status, int mode = 2);
/*do log statistics with time interval*/
static int do_nginx_log_stats(log_item const& item, std::unordered_map<std::string, nginx_domain_stat> & logstats);

/*load devicelist, @param devicelist map<ip, device_id>*/
static int load_devicelist(char const* file, std::unordered_map<std::string, int>& devicelist);

/*get device_id by ip*/
static int get_device_id(std::unordered_map<std::string, int> const& devicelist);

/*read domain from log_file*/
static char const * find_domain(char const * nginx_log_file);
/* url manage:
 * if @param str NOT exist, then add as new, else return exist
 * this funtions currently NOT used because of poor speedup, @date: 2016/10/27
 */
static char const * str_find(char const *str, int len = -1);

/*nginx_log_analysis/print_table.cpp*/
extern int print_nginx_log_stats(std::unordered_map<std::string, nginx_domain_stat> const& logstats);
//////////////////////////////////////////////////////////////////////////////////

/*GLOBAL vars*/
/*nginx_log_analysis/option.cpp*/
extern struct nla_options nla_opt;
/*map<ip_addr : device_id>*/
static std::unordered_map<std::string, int> g_devicelist;
/*map<domain, site_info>*/
static std::unordered_map<std::string, site_info> g_sitelist;
static size_t g_line_count = 0;
time_t g_start_time = 0;
int g_device_id = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
static char * parse_nginx_log_request_uri_url(char * request_uri, int * len, char const * cache_status, int mode/* = 2*/)
{
	auto url = strchr(request_uri, '/'); /*"GET /abc?param1=abc"*/
	if(!url) return NULL;
	auto pos = strchr(url, ' ');
	auto m = nla_opt.parse_url_mode;
	if(m == 1){
		auto p = strchr(url, '?');
		if(p) pos = p;
	}
	else if(m == 2){
		auto miss0 = strrchr(cache_status, '0');
		auto pos1 = strchr(url, '?'), pos2 = strchr(url, ';');
		if(miss0 && (pos1 || pos2)){
			if(pos1 && pos2)
				pos = std::min(pos1, pos2);
			else{
				pos = (pos1? pos1 : pos2);
			}
		}
	}
	auto length = pos - url;
	url[length] = '\0';
	if(len)
		*len = length;
	return url;
}

static int parse_log_item(log_item & item, char *& logitem, char delim /*= '\0'*/)
{
	memset(&item, 0, sizeof(log_item));
	char *items[18];
	int result = do_parse_nginx_log_item(items, logitem, delim);
	if(result != 0){
		return 1;
	}

	item.domain = items[0];
//	item.client_ip_2 = items[1];
	item.client_ip = netutil_get_ip_from_str(items[1]);
	if(item.client_ip == 0)
		return 1;

	char * end;
	item.request_time = strtoul(items[2], &end, 10);
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	tm my_tm;
	if(!strptime(items[4] + 1, "%d/%b/%Y:%H:%M:%S" , &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	item.time_local = mktime(&my_tm);

	auto url = parse_nginx_log_request_uri_url(items[6], NULL, items[3]);
	if(!url) return -1;
	item.request_url = url;

	char const * p = items[8];
	item.bytes_sent = strtoul(p, &end, 10);
	item.status = atoi(items[7]);
	item.is_hit = (strcmp(items[3],"HIT") == 0);
	return 0;
}

static int do_nginx_log_stats(log_item const& item, std::unordered_map<std::string, nginx_domain_stat> & logstats)
{
	auto & dstat = logstats[item.domain];
	if(dstat._site_id == 0)
		find_site_id(g_sitelist, item.domain, dstat._site_id, &dstat._user_id);

	auto & logsstat = dstat._stats[item.time_local];
	if(!item.is_hit){
		logsstat._bytes_m += item.bytes_sent;
		++logsstat._access_m[item.status];
	}

	if(nla_opt.output_file_flow || nla_opt.output_file_url_popular || nla_opt.output_file_http_stats){
		char buff[strlen(item.request_url)];
		sha1sum_r(item.request_url, sizeof(buff), buff);
		url_stat& urlstat = logsstat._url_stats[buff];
		++urlstat._status[item.status];
		urlstat._bytes[item.status] += item.bytes_sent;
	}

	/*if NOT required, we needn't statistics it*/
	if(nla_opt.output_file_ip_popular || nla_opt.output_file_ip_slowfast){
		ip_stat& ipstat =logsstat._ip_stats[item.client_ip];
		ipstat.bytes += item.bytes_sent;
		ipstat.sec += item.request_time;
		++ipstat.access;

	}
	if(nla_opt.output_file_cutip_slowfast){
		auto & cutipstat = logsstat._cuitip_stats[item.client_ip];
		cutipstat.bytes += item.bytes_sent;
		cutipstat.sec += item.request_time;
	}
	if(nla_opt.output_file_ip_source){
		/*FIXME, @date 2016/11/11*/
//		if(nla_opt.enable_devicelist_filter &&  g_devicelist[item.client_ip_2] != 0)
//			return 0;
		locisp_stat& listat = logsstat._locisp_stats[item.client_ip];
		listat.bytes += item.bytes_sent;
		++listat.access;
		if(!item.is_hit){
			listat.bytes_m += item.bytes_sent;
			++listat.access_m;
		}
	}
	return 0;
}

int load_devicelist(char const* file, std::unordered_map<std::string, int>& devicelist)
{
	if(!file) return -1;
	FILE * f = fopen(file, "r");
	if(!f) {
		fprintf(stderr, "%s: fopen file %s failed\n", __FUNCTION__, file);
		return 1;
	}
	char data[1024] = "";
	while(fgets(data, sizeof(data), f)){
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
			if(nla_opt.verbose)
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

	log_item item;
	for(char * p = buf; p != buf + len; ++p){
		int result = parse_log_item(item, p, '\n');
		if(result == 0){
			do_nginx_log_stats(item, logstats);
		}
		else {
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

static int log_stats_append(std::unordered_map<std::string, nginx_domain_stat> & a,
		std::unordered_map<std::string, nginx_domain_stat> const& b)
{
	for(auto const& item : b){
		for(auto const &stat : item.second._stats)
			a[item.first]._stats[stat.first] += stat.second;
	}
	return 0;
}

static int parallel_parse_nginx_log(FILE * f, std::unordered_map<std::string, nginx_domain_stat> & stats)
{
	struct stat logfile_stat;
	if(fstat(fileno(f), &logfile_stat) < 0){
		fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "nginx_log_file");
		return 1;
	}
	int parallel_count = get_nprocs() - 1;			/*parallel count, exclude main_thread, maybe 0*/
	size_t min_bytes = 1024 * 1024 * 64; 	/*min 64MB*/
	for(size_t c = logfile_stat.st_size / min_bytes; c < (size_t)parallel_count; --parallel_count){ /*empty*/ }
	/*!
	 * FIXME:
	 * 1. be aware of local static vars when in multi-thread
	 * 2. disabled by default, because of little speedup
	 */
	if(!nla_opt.enable_multi_thread)
		parallel_count = 0;
	if(nla_opt.verbose)
		fprintf(stdout, "%s: logfile_size = %ld/%s, para_count=%d\n", __FUNCTION__
				, logfile_stat.st_size, byte_to_mb_kb_str(logfile_stat.st_size, "%-.2f %cB"), parallel_count);

	char  * start_p = (char *)mmap(NULL, logfile_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f), 0);
	if(!start_p || start_p == MAP_FAILED){
		fprintf(stderr, "%s: mmap() failed for %s\n", __FUNCTION__, "nginx_log_file");
		return 1;
	}
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
		if(nla_opt.verbose){
			char buff[64];
			printf("%s: worker_thread=%zu, process size=%zu/%s, percent=%.1f%%\n", __FUNCTION__,
					tid, nlen, byte_to_mb_kb_str_r(nlen, "%-.2f %cB", buff), (double)nlen * 100/ logfile_stat.st_size);
		}

		threads[i] = tid;
		offset_p += nlen;
	}
	size_t left_len = logfile_stat.st_size - offset_p;
	if(nla_opt.verbose){
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
		if(nla_opt.verbose)
			fprintf(stdout, "%s: thread=%zu exited\n", __FUNCTION__, item);
		auto & ct = *(parse_context * )tret;
		log_stats_append(stats, ct.logstats);
		g_line_count += ct.total_lines;
	}
	if(parallel_count == 0){
		/*FIXME: NOT needed? just for speed*/
		stats = parse_args[parallel_count].logstats;
	}
	else{
		log_stats_append(stats, parse_args[parallel_count].logstats);
	}
	g_line_count += parse_args[parallel_count].total_lines;
	if(nla_opt.verbose)
		fprintf(stdout, "%s: processed, total_line: %-8zu\n", __FUNCTION__, g_line_count);

	munmap(start_p, logfile_stat.st_size);
	return 0;
}

int test_nginx_log_stats_main(int argc, char ** argv)
{
//	test_nginx_log_analysis_main(argc, argv);	/*for test only*/

	int result = nginx_log_stats_parse_options(argc, argv);
	if(result != 0 || nla_opt.show_help){
		nla_opt.show_help? nginx_log_stats_show_help(stdout) :
				nginx_log_stats_show_usage(stdout);
		return 0;
	}
	if(nla_opt.verbose)
		nla_options_fprint(stdout, &nla_opt);
	if(nla_opt.print_device_id){	//query device_id and return
		int result = load_devicelist(nla_opt.devicelist_file, g_devicelist);
		int id = result == 0? get_device_id(g_devicelist) : 0;
		fprintf(stdout, "%d\n", id);
		return result == 0? 0 : 1;
	}

	g_start_time = time(NULL);
	result = load_devicelist(nla_opt.devicelist_file, g_devicelist);
	if(result != 0){
		fprintf(stderr, "%s: load_devicelist() failed\n", __FUNCTION__);
		return 1;
	}
	result = load_sitelist(nla_opt.siteuidlist_file, g_sitelist);
	if(result != 0){
		fprintf(stderr, "%s: load_sitelist() failed\n", __FUNCTION__);
		return 1;
	}
#ifdef ENABLE_IPMAP
	if(nla_opt.output_file_ip_source && 0 != locisp_group::load_ipmap_file(nla_opt.ipmap_file))  {
		fprintf(stderr, "%s: ipmap_load(%s) failed\n", __FUNCTION__, nla_opt.ipmap_file);
		return 1;
	}
#else
	fprintf(stdout, "%s: ipmap DISABLED on this platform\n", __FUNCTION__);
#endif //ENABLE_IPMAP

	auto nginx_log_file = fopen(nla_opt.log_file, "r");
	if(!nginx_log_file) {
		fprintf(stderr, "fopen file %s failed\n", nla_opt.log_file);
		return 1;
	}

	int interval = nla_opt.interval;
	if(interval < 300 || interval > 3600){
		fprintf(stdout, "%s: WARNING, interval(%d) too %s\n", __FUNCTION__, interval, interval < 300? "small" : "large");
	}
	time_group::_sec = interval;
	g_device_id = nla_opt.device_id > 0? nla_opt.device_id : get_device_id(g_devicelist);
	/*parse logs*/
	std::unordered_map<std::string, nginx_domain_stat> logstats;	/*[domain: domain_stat]*/
	parallel_parse_nginx_log(nginx_log_file, logstats);

	/*output results*/
	print_nginx_log_stats(logstats);

	return 0;
}
