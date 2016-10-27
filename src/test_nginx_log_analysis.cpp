/*!
 * This file is PART of nginx_log_analysis
 * parse nginx logs and print results
 * @note: 1.As of current design, the input nginx log file MUST have ONLY one domain, @see test_nginx_log_split_main
 */
#include <stdio.h>
#include <string.h> 	/*strncpy*/
#include <fnmatch.h>	/*fnmatch*/
#include <sys/sysinfo.h>	/*get_nprocs*/
#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
//#include <locale.h> 	/*setlocale*/
#include <pthread.h> 	/*pthread_create*/

#include <unordered_map> 	/*std::unordered_map*/
#include <map>				/*std::map*/
#include "bd_test.h"		/*test_nginx_log_stats_main*/
#include "test_options.h"	/*nla_options**/
#include "nginx_log_analysis.h"	/*log_stats, ...*/
#include "string_util.h"	/*md5sum*/
#include "net_util.h"	/*get_if_addrs, ...*/

/*tests, @see nginx_log_analysis2.cpp*/
extern int test_nginx_log_analysis_main(int argc, char ** argv);
/*!
 * parse ' ' splitted nginx log
 * nginx_log sample:
 * flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] \
 * "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" \
 * "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E

 * @NOTE:current nginx_log format:
 * $host $remote_addr $request_time_msec $cache_status [$time_local] "$request_method $request_uri $server_protocol" $status $bytes_sent \
 * "$http_referer" "$remote_user" "$http_cookie" "$http_user_agent" $scheme $request_length $upstream_response_time'
 * total fields == 18
 * */
static int do_parse_log_item(char ** fields, char *& szitem, char delim = '\0');
/*parse nginx_log buffer @apram ct, and output results*/
static int parse_log_item_buf(parse_context& ct);
/*split file @param f into parts, use pthread to parallel parse */
static int parallel_parse(FILE * f, std::map<time_interval, log_stat> & stats);

/*log statistics with time interval*/
static int log_stats(time_interval & m, log_item const& item, std::map<time_interval, log_stat>& logstats);

/*load devicelist, @param devicelist map<device_id, ip>*/
static int load_devicelist(char const* file, std::unordered_map<int, char[16]>& devicelist);
/*load sitelist*/
static int load_sitelist(char const* file, std::unordered_map<std::string, site_info>& sitelist);

/*find site_id by site_name/domain*/
static int find_site_id(std::unordered_map<std::string, site_info> const& sitelist, const char* site, int & siteid, int * user_id = NULL);
/*get device_id by ip*/
static int get_device_id(std::unordered_map<int, char[16]> const& devicelist);

/*read domain from log_file*/
static char const * find_domain(char const * nginx_log_file);
/* url manage:
 * if @param str NOT exist, then add as new, else return exist
 * this funtions currently NOT used because of poor speedup, @date: 2016/10/27
 */
static char const * str_find(char const *str, int len = -1);

/*nginx_log_analysis_print_table.cpp*/
extern int print_stats(std::map<time_interval, log_stat>const& logstats,
		int device_id, int site_id, int user_id);
//////////////////////////////////////////////////////////////////////////////////

/*GLOBAL vars*/
/*all options: test_options.cpp*/
extern struct nla_options nla_opt;
/*map<device_id, ip_addr>*/
static std::unordered_map<int, char[16]> g_devicelist;
/*map<domain, site_info>*/
static std::unordered_map<std::string, site_info> g_sitelist;
static size_t g_line_count = 0;

///////////////////////////////////////////////////////////////////////////////////////////////
static int parse_log_item(log_item & item, char *& logitem, char delim /*= '\0'*/)
{
	memset(&item, 0, sizeof(log_item));
	char *items[18];
	int result = do_parse_log_item(items, logitem, delim);
	if(result != 0){
		return 1;
	}
	item.client_ip = netutil_get_ip_from_str(items[1]);
	if(item.client_ip == 0)
		return 1;
	item.request_time = atoi(items[2]);
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	tm my_tm;
	if(!strptime(items[4] + 1, "%d/%b/%Y:%H:%M:%S" , &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	item.time_local = mktime(&my_tm);
	char * url = strchr(items[6], '/'); /*"GET /abc?param1=abc"*/
	int len = strchr(url, ' ') - url;
	url[len] = '\0';

	/*!
	 * FIXME: str_find NOT correct in multi-thread, and little speedup, disabled, @date 2016/10/27
	 */
	char buff[33];
	item.request_url = md5sum_r(url, len, buff);

	char const * p = items[8];
	char * end;
	item.bytes_sent = strtoul(p, &end, 10);
	item.status = atoi(items[7]);
	item.is_hit = (strcmp(strupr(items[3]),"HIT") == 0);
	return 0;
}

static int log_stats(time_interval & m, log_item const& item, std::map<time_interval, log_stat>& logstats)
{
	m.mark(item.time_local);
	log_stat& logsstat = logstats[m];

	url_stat& urlstat = logsstat._url_stats[item.request_url];
	++urlstat._status[item.status];
	urlstat._bytes[item.status] += item.bytes_sent;

	ip_stat& ipstat =logsstat._ip_stats[item.client_ip];
	ipstat.bytes += item.bytes_sent;
	ipstat.sec += item.request_time;
	++ipstat.access;

	if(!item.is_hit){
		logsstat._bytes_m += item.bytes_sent;
		++logsstat._access_m;
	}
	return 0;
}

static int load_sitelist(char const* file, std::unordered_map<std::string, site_info>& sitelist)
{
	if(!file || file[0] == '\0') return -1;
	FILE * f = fopen(file, "r");
	if(!f){
		fprintf(stderr, "%s: fopen file %s failed\n", __FUNCTION__, file);
		return -1;
	}
	char data[1024] = "";
	while(fgets(data, sizeof(data), f)){
		data[strlen(data) - 1] = '\0';

		site_info sitel;
		char const * token = strtok(data, " ");
//		fprintf(stdout, "[%d: %s]", i, token);
		sitel.site_id = atoi(token);

		token = strtok(NULL, " ");
		sitel.user_id = atoi(token);

		token = strtok(NULL, " ");
		sitelist[token] = sitel;
	}
	return 0;
}

static int find_site_id(std::unordered_map<std::string, site_info> const& sitelist, const char* site, int & siteid, int * user_id)
{
	if(!site || site[0] == '\0')
		return -1;

	site_info  const * si = NULL;

	if(sitelist.count(site) != 0){
//		fprintf(stdout, "%s: FULL matched\n", __FUNCTION__);
		si = &sitelist.at(site);
	}
	else{
		for(auto const & item : sitelist){
			if(fnmatch(item.first.c_str(), site, 0) == 0){
	//			fprintf(stdout, "%s: WILDCARD matched, pattern=%s\n", __FUNCTION__, it->first.c_str());
				si = &item.second;
				break;
			}
		}
	}
	siteid = si? si->site_id : 0;
	if(user_id)
		*user_id = si? si->user_id : 0;
	return 0;
}

int load_devicelist(char const* file, std::unordered_map<int, char[16]>& devicelist)
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
		strncpy(devicelist[id], token, 15);
	}
//	for(auto const& item : devicelist){
//		fprintf(stdout, "%d--%s\n", item.first, item.second);
//	}
	return 0;
}

static int get_device_id(std::unordered_map<int, char[16]> const & devicelist)
{
	char ips[64][16];
	int count = 64;
	int result  = get_if_addrs(ips[0], count, 16);
	if(result != 0)
		return -1;
	//std::find_first_of();
	for(int i = 0; i < count; ++i){
		for(auto const& item : devicelist){
			if(strcmp(item.second, ips[i]) == 0)
				return item.first;
		}
	}
	return 0;
}

static char const * find_domain(char const * nginx_log_file)
{
	static char domain[512] = "<error domain>";
	FILE * f = fopen(nginx_log_file, "r");
	if(!f) {
		fprintf(stderr, "%s: fopen file %s failed\n", __FUNCTION__, nginx_log_file);
		return domain;
	}
	char data[512] = "";
	if(!fgets(data, sizeof(data), f))
		return domain;
	int len = strchr(data, ' ') - data;
	strncpy(domain, data, len);
	domain[len] = '\0';
	fclose(f);

	return domain;
}

int do_parse_log_item(char** fields, char*& szitem, char delim/* = '\0'*/)
{
//	for(char * ch = szitem; ; ++ch) { fprintf(stdout, "%c", *ch); if(*ch == delim) break; }
	bool arg_start = false;
	int field_count = 0;

	char * q = szitem;
	for(char * p = szitem; ; ++q){
		if(*q == '"'){
			if(!arg_start) {
				arg_start = true;
				p = q + 1;
			}
			else{
				arg_start = false;
				if(!(*(q + 1) == ' ' || *(q + 1) == delim)){
//					fprintf(stderr, "%s: parse error at %s\n", __FUNCTION__, q);
					goto error_return;
				}
				*q = '\0';
				fields[field_count++] = p;
				++q;
				if(*q == delim)
					break;
				p = q + 1;
			}
			continue;
		}
		if(arg_start && *q == delim){
//			fprintf(stderr, "%s: parse error\n", __FUNCTION__);
			goto error_return;
		}
		if(!arg_start && (*q == ' ' || *q == delim)){
			fields[field_count++] = p;
			char c = *q;
			*q = '\0';
			if(c == delim){
				break;
			}
			p = q + 1;
		}
	}
	szitem = q;
//	pthread_mutex_lock(&g_io_mutex);
//	for(int i  = 0; i < field_count; ++i){
//		fprintf(stdout, "%s: argv[%02d]: %s\n", __FUNCTION__, i, fields[i]);
//	}
//	pthread_mutex_unlock(&g_io_mutex);
	return 0;
error_return:
	fields[0] = '\0';
	return -1;
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
	if(!str || str[0] == '\0')
		return NULL;
	/*FIXME: dangerous in multi-threaded!!! */
	static std::unordered_map<std::string, char *> urls;
	static size_t step = 10 * 1024, total = 1024 * 64;	/*KB*/
	static char * start_p = (char *)malloc(total);
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

int parse_log_item_buf(parse_context& ct)
{
	auto & buf = ct.buf;
	auto & len = ct.len;
	auto & logstats = ct.logstats;
	auto & total_lines = ct.total_lines = 0;

	time_interval m;
	log_item item;
	for(char * p = buf; p != buf + len; ++p){
		int result = parse_log_item(item, p, '\n');
		if(result == 0){
			log_stats(m, item, logstats);
		}
		else {
			//current line failed, move to next line
			while(p != buf + len && !(*p == '\0' || *p == '\n')) { ++p; }
		}
		++total_lines;
	}
	return 0;
}

void * parallel_parse_thread_func(void * varg)
{
	if(!varg) return varg;
	auto & arg = *(parse_context*)varg;
	/*arg->buf, arg->len, arg->logstats, arg->total_lines, arg->failed_lines*/
	parse_log_item_buf(arg);

	return varg;
}

static int log_stats_append(std::map<time_interval, log_stat> & a, std::map<time_interval, log_stat> const& b)
{
	for(auto const& item : b){
		a[item.first] += item.second;
	}
	return 0;
}

static int parallel_parse(FILE * f, std::map<time_interval, log_stat> & stats)
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
	if(!nla_opt.enalbe_multi_thread)
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
	parse_log_item_buf(arg);

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
		stats.swap(parse_args[parallel_count].logstats);
	}
	else{
		log_stats_append(stats, parse_args[parallel_count].logstats);
	}
	g_line_count += parse_args[parallel_count].total_lines;
	munmap(start_p, logfile_stat.st_size);
	return 0;
}

int test_nginx_log_stats_main(int argc, char ** argv)
{
	test_nginx_log_analysis_main(argc, argv);
	int result = nginx_log_stats_parse_options(argc, argv);

	if(result != 0 || nla_opt.show_help){
		nla_opt.show_help? nginx_log_stats_show_help(stdout) :
				nginx_log_stats_show_usage(stdout);
		return 0;
	}
	if(nla_opt.verbose)
		nla_options_fprint(stdout, &nla_opt);
	//	setlocale(LC_NUMERIC, "");
	if(nla_opt.show_device_id){	//query device_id and return
		int result = load_devicelist(nla_opt.devicelist_file, g_devicelist);
		int id = result == 0? get_device_id(g_devicelist) : 0;
		fprintf(stdout, "%d\n", id);
		return result == 0? 0 : 1;
	}

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
	FILE * nginx_log_file = fopen(nla_opt.log_file, "r");
	if(!nginx_log_file) {
		fprintf(stderr, "fopen file %s failed\n", nla_opt.log_file);
		return 1;
	}

	int interval = nla_opt.interval;
	if(interval < 300 || interval > 3600){
		fprintf(stdout, "%s: WARNING, interval(%d) too %s\n", __FUNCTION__, interval, interval < 300? "small" : "large");
	}
	time_interval::_sec = interval;

	int device_id = 0, site_id = 0, user_id  = 0;
	device_id = nla_opt.device_id > 0? nla_opt.device_id : get_device_id(g_devicelist);
	find_site_id(g_sitelist, find_domain(nla_opt.log_file), site_id, &user_id);
	if(nla_opt.verbose)
		fprintf(stdout, "%s: device_id=%d, site_id=%d, user_id=%d\n", __FUNCTION__,
			device_id, site_id, user_id);

	std::map<time_interval, log_stat> logstats;
	parallel_parse(nginx_log_file, logstats);
	if(nla_opt.verbose)
		fprintf(stdout, "\r%s: processed, total_line: %-8ld\n", __FUNCTION__, g_line_count);
	print_stats(logstats, device_id, site_id, user_id);

	/*free*/
	str_find(" ", -2);
	return 0;
}
