#include "bd_test.h"
#include <time.h>	/*time_t, strptime*/
#include <stdio.h>
#include <string.h> /*strncpy*/
#include <pthread.h> /*pthread*/
#include <math.h> /*pow*/
#include <assert.h>
#include <fnmatch.h>	/*fnmatch*/
#include <sys/sysinfo.h>	/*get_nprocs*/
#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
#include <vector>
#include <algorithm> /*std::sort*/
#include <unordered_map> /*unordered_map*/
#include <numeric>	/*std::accumulate*/
#include <map>
/*declares*/
struct log_item;
struct site_info;
struct parallel_parse_thread_arg;
struct url_count;
class time_mark;
class url_stat;
class log_stat;

static int test_strptime_main(int argc, char ** argv);
static int test_time_mark_main(int argc, char ** argv);
static int test_get_if_addrs_main(int argc, char ** argv);
/*test_mem.cpp*/
extern int test_alloc_mmap_main(int argc, char * argv[]);

/*parse a nginx log*/
static int parse_log_item(log_item & item, char const * logitem, char delim = '\0');
static int log_stats(time_mark & m, log_item const& item, std::map<time_mark, log_stat>& logstats);
/*print ouput*/
static int print_stats(FILE * stream, std::map<time_mark, log_stat> const& stats, int top = 10);
static void print_flow_table(FILE * stream, std::map<time_mark, log_stat> const& stats,
		int device_id, int site_id, int user_id);
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
 * if @param str NOT exist, then add as new, else return exist*/
static char const * str_find(char const *str, int len = -1);

/*string_util.cpp*/
extern char const * md5sum(char const * str, int len);
/*net_util.cpp*/
extern int get_if_addrs(char *ips, int & count, int sz);
//////////////////////////////////////////////////////////////////////////////////
struct log_item{
	time_t time_local;
	char const *request_url;
	double bytes_sent;
	int status;
	bool is_hit;
};
//////////////////////////////////////////////////////////////////////////////////
struct site_info{
	int site_id;
	int user_id;
};
//////////////////////////////////////////////////////////////////////////////////
struct parallel_parse_thread_arg
{
	char const * buf;
	size_t len;
	std::map<time_mark, log_stat> logstats;
	std::vector<long> failed_lines;
};

/*GLOBAL vars*/
/*map<device_id, ip_addr>*/
static std::unordered_map<int, char[16]> g_devicelist;
static std::unordered_map<std::string, site_info> g_sitelist;
static double g_line_count = 0;
static pthread_mutex_t g_io_mutex;
static pthread_mutex_t g_line_count_mutex;
//////////////////////////////////////////////////////////////////////////////////
class time_mark{
public:
	static int _interval_sec;	/*in seconds*/
private:
	time_t _t;
public:
	explicit time_mark(char const * strtime = NULL);
public:
	operator bool() const;
	time_mark next_mark() const;
	time_mark& mark(char const * strtime);
	time_mark& mark(time_t const& t);
	char const * c_str(char const * fmt = "%Y-%m-%d %H:%M:%S") const;
private:
	friend bool operator ==(const time_mark& one, const time_mark& another);
	friend bool operator <(const time_mark& one, const time_mark& another);
};

int time_mark::_interval_sec = 300;
time_mark::time_mark(char const * strtime)
: _t(0)
{
	if(strtime)
		mark(strtime);
}

time_mark::operator bool() const
{
	return 1;
}

const char* time_mark::c_str(char const * fmt) const
{
//	return ctime(&_t);
	static char buff[20] = "";
	if(!fmt)
		return "<null time_mark>";
	strftime(buff, 20, fmt, localtime(&_t));
	return buff;
}

time_mark& time_mark::mark(const char* strtime)
{
	if(!strtime) return *this;
	tm my_tm;
	char const * result = strptime(strtime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	if(!result) return *this;

	my_tm.tm_isdst = 0;
	time_t t = mktime(&my_tm);
//	fprintf(stdout, "%s: %s, t=%ld,_t=%ld\n", __FUNCTION__, ctime(&t), t, _t);
	return mark(t);
}

time_mark& time_mark::mark(time_t const& t)
{
	long n = difftime(t, _t) / _interval_sec;
	_t += n * _interval_sec;
	return *this;
}
bool operator <(const time_mark& one, const time_mark& another)
{
	return one._t < another._t;
}

bool operator ==(const time_mark& one, const time_mark& another)
{
	return one._t == another._t;
}

time_mark time_mark::next_mark() const
{
	time_mark ret(*this);
	ret._t += _interval_sec;
	return ret;
}


//////////////////////////////////////////////////////////////////////////////////
class url_stat{
public:
	std::unordered_map<int, long> _status;		/*http_status_code: count*/
	std::unordered_map<int, double> _bytes;	/*http_status_code: bytes*/
public:
	long access_total() const;
	double bytes(int code1, int code2 = -1) const;
	double bytes_total() const;
	long access(int code) const;
public:
	url_stat& operator+=(url_stat const& another);
};

long url_stat::access_total() const
{
	long ret  = 0;
	for(auto it = _status.begin(); it !=_status.end(); ++it){
		ret += it->second;
	}
	return ret;
}

long url_stat::access(int code) const
{
	return (_status.count(code) != 0? _status.at(code) : 0);
}

inline double url_stat::bytes(int code1, int code2/* = -1*/) const
{
	double ret  = 0;
	for(auto it = _bytes.begin(); it !=_bytes.end(); ++it){
		if(it->first == code1 || it->first == code2)
			ret += it->second;
	}
	return ret;

}

double url_stat::bytes_total() const
{
	double ret  = 0;
	for(auto it = _bytes.begin(); it !=_bytes.end(); ++it){
		ret += it->second;
	}
	return ret;
}

url_stat& url_stat::operator+=(url_stat const& another)
{
	for(auto const& item : another._status){
		_status[item.first] += item.second;
	}
	for(auto const& item : another._bytes){
		_bytes[item.first] += item.second;
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////
class log_stat{
public:
	std::unordered_map<char const *, url_stat> _url_stats;	/*url:url_stat*/
	double _bytes_m;
	long _access_m;
public:
	log_stat();
public:
	double bytes_total() const;
	double bytes(int code1, int code2 = -1) const;
	long access_total() const;
	long access(int code) const;
};

log_stat::log_stat()
: _bytes_m(0)
, _access_m(0)
{
	//none
}

long log_stat::access_total() const
{
	long ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access_total();
	}
	return ret;
}

inline double log_stat::bytes(int code1, int code2/* = -1*/) const
{
	double ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes(code1, code2);
	}
	return ret;
}

inline double log_stat::bytes_total() const
{
	double ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes_total();
	}
	return ret;
}
long log_stat::access(int code) const
{
	long ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access(code);
	}
	return ret;
}

/*
 * parse ' ' splitted logitem, nginx log:
 * flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] \
 * "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" \
 * "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E
 */
int do_parse_log_item(int * count, char *** items, char const * szitem, char delim = '\0')
{
//	fprintf(stdout, "item=%s\n", szitem);
	static char * s_items[20] = {0};
	*items= s_items;

	bool arg_start = false;
	int c = 0;
	for(char const * p = szitem, *q = p; ; ++q){
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
				++q;
				char * arg = new char[q - p];
				strncpy(arg, p, q - p - 1);
				arg[q - p - 1] = '\0';
				s_items[c++] = arg;
				if(*q == delim)
					break;
				p = q + 1;
			}
			continue;
		}
		if(arg_start && *q != delim){
//			fprintf(stderr, "%s: parse error\n", __FUNCTION__);
			goto error_return;
		}
		if(!arg_start && (*q == ' ' || *q != delim)){
			char * arg = new char[q - p + 1];
			strncpy(arg, p, q - p);
			arg[q - p] = '\0';
			s_items[c++] = arg;
			if(*q == delim)
				break;
			p = q + 1;
		}
	}
	*count = c;
//	for(int i  = 0; i < *count; ++i){
//		fprintf(stdout, "argv[%d]: %s\n", i, s_items[i]);
//	}
	return 0;
error_return:
	*count = 0;
	s_items[0] = '\0';
	return -1;
}

static int parse_log_item(log_item & item, char const * logitem, char delim /*= '\0'*/)
{
	memset(&item, 0, sizeof(log_item));
	int count;
	static char ** items;
	int result = do_parse_log_item(&count, &items, logitem, delim);
	if(result != 0){
		return 1;
	}
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	char time_local[21];
	strncpy(time_local, items[4] + 1, 20);
	tm my_tm;
	if(!strptime(time_local, "%d/%b/%Y:%H:%M:%S" , &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	item.time_local = mktime(&my_tm);

	char const * url = strchr(items[6], '/'); /*"GET /abc?param1=abc"*/
	int len = strchr(url, ' ') - url;
	item.request_url = str_find(url, len);

	char const * p = items[8];
	char * end;
	item.bytes_sent = strtod(p, &end);
	item.status = atoi(items[7]);
	item.is_hit = (strcmp(strupr(items[3]),"HIT") == 0);
	return 0;
}

struct url_count
{
	char const * url;
	long count;
};

bool compare_by_access_count(url_count const& a, url_count const& b)
{
	return a.count > b.count;
}
static void url_top_n(std::map<time_mark, log_stat> const& stats, std::vector<url_count>& urlcount)
{
	std::unordered_map<char const *, url_stat> urlstats;
	for(auto it = stats.begin(); it != stats.end(); ++it){
		auto const& urlstat = it->second._url_stats;
		for(auto const& item : urlstat){
			urlstats[item.first] += item.second;
		}
	}
	for(auto const& item : urlstats){
		url_count c = {item.first, item.second.access_total()};
		urlcount.push_back(c);
	}
	std::sort(urlcount.begin(), urlcount.end(), compare_by_access_count);
}

static int print_stats(FILE* stream, const std::map<time_mark, log_stat>& stats, int top)
{
	fprintf(stream, "%-20s%-120s%-10s%-20s\n", "Time", "Url", "Count", "Bytes");
	double bytes_total = 0, bytes_200 = 0, bytes_206 = 0, other_bytes = 0;
	long access_total = 0, access_200 = 0, access_206 = 0;
	for(auto it = stats.begin(); it != stats.end(); ++it){
		log_stat const& stat = it->second;
		auto total = stat.bytes_total(), b200 = stat.bytes(200), b206 = stat.bytes(206);
		bytes_total += total;
		bytes_200 += b200;
		bytes_206 += b206;
		other_bytes += (total - b200 - b206);
		access_total += stat.access_total();
		access_200 += stat.access(200);
		access_206 += stat.access(206);

//		fprintf(stream, "%-20s%-120s%-10s%-20.0f\n", it->first.c_str(), " ", " ", stat.bytes_total());
//		if(top > 0 && --top <= 0)
//			break;
//
//		for(auto it2 = stat._url_stats.begin(); it2 != stat._url_stats.end(); ++it2){
//			fprintf(stream, "%-20s%-120s%-10ld%-20s\n", " ", it2->first, it2->second.access_total(), " ");
//		}
	}
	fprintf(stream, "%-15s%-15s%-20s%-20s%-20s%-20s%-20s%-40s\n", "access_total", "access_200", "access_206",
			"bytes_200", "bytes_206", "200+206", "other", "Total Byte/GB");
	fprintf(stream, "%-15ld%-15ld%-20ld%-20.6f%-20.6f%-20.6f%-20.6f%-20.6f/%-20.6f\n",
			access_total, access_200, access_206, bytes_200,
			bytes_206, bytes_200 + bytes_206, other_bytes, bytes_total, bytes_total / pow(1024.0, 3));

	std::vector<url_count> urlcount;
	url_top_n(stats, urlcount);
	fprintf(stream, "%-100s%-70s\n", "url", "count");
	int left = 10;
	for(auto it = urlcount.begin(); it != urlcount.end() && left > 0; ++it){
		fprintf(stream, "%-100s%-70ld\n", it->url, it->count);
		--left;
	}
	return 0;
}

static void print_flow_table(FILE * stream, std::map<time_mark, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		log_stat const& stat = item.second;
		char line[512];
		int sz = snprintf(line, 512, "%d %s %d %ld %.0f %d %ld %.0f\n",
				site_id, item.first.c_str("%Y%m%d%H%M"), device_id, stat.access_total()
				, stat.bytes_total(), user_id, stat._access_m, stat._bytes_m);
		if(sz <= 0){
			++i;
			continue;
		}
		buff += line;
		if(buff.size() > 1024 * 1024 * 16){
			size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
			if(r != buff.size())
				fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
						__FUNCTION__, buff.size(), r);
			buff.clear();
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline int log_stats(const std::vector<log_item>& logvec,
		std::map<time_mark, log_stat>& logstats)
{
	time_mark m;
	for(log_item const& item : logvec){
		m.mark(item.time_local);
		log_stat& logsstat = logstats[m];

		url_stat& urlstat = logsstat._url_stats[item.request_url];
		++urlstat._status[item.status];
		urlstat._bytes[item.status] += item.bytes_sent;

	}
	return 0;
}

static int log_stats(time_mark & m, log_item const& item, std::map<time_mark, log_stat>& logstats)
{
	m.mark(item.time_local);
	log_stat& logsstat = logstats[m];

	url_stat& urlstat = logsstat._url_stats[item.request_url];
	++urlstat._status[item.status];
	urlstat._bytes[item.status] += item.bytes_sent;

	if(!item.is_hit){
		logsstat._bytes_m += item.bytes_sent;
		++logsstat._access_m;
	}
	return 0;
}

static int test_time_mark_main(int argc, char ** argv)
{
	char const * stime = "17/Sep/2016:01:19:43";
	time_t time1 = time(NULL);
	tm time2 = *localtime(&time1);
	time2.tm_hour -= 1;
	assert((time1 - 3600) == mktime(&time2));
	fprintf(stdout, "time=%ld, difftime(0, time)=%ld\n", time1, (long)difftime((time_t)0, time1));

	time_mark timem1(stime), timem2(stime);
	assert(timem1);
	assert(timem1 == timem2);
	assert(timem1 == time_mark("17/Sep/2016:01:19:00"));
	fprintf(stdout, "time=17/Sep/2016:01:19:23,mark=%s\n", time_mark().mark("17/Sep/2016:01:19:23").c_str());
	fprintf(stdout, "time=%s, start=%s\n", stime, timem1.c_str());
	for(int i= 0; i < 5; ++i){
		fprintf(stdout, "next=%s\n", timem1.next_mark().c_str());
		timem1 = timem1.next_mark();
	}
	time_mark startm("17/Sep/2016:01:19:43"), timem3(startm);
	assert(startm == timem3.mark("17/Sep/2016:01:15:43"));
	return 0;
}
static int test_strptime_main(int argc, char ** argv)
{
	fprintf(stdout, "%s:\n", __FUNCTION__);
	struct tm my_tm;
	char const * stime = "17/Sep/2016:08:19:43";
	char const * result = strptime(stime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	my_tm.tm_isdst = 0;
	mktime(&my_tm);
	fprintf(stdout, "time=%s, NOT paused=[%s], asctime:sec=%d,all=%s\n",
			stime, result? result : "<null>", my_tm.tm_sec, asctime(&my_tm));
	return 0;
}

int test_nginx_log_split_main(int argc, char ** argv)
{
	if(argc < 3){
		fprintf(stderr, "split nginx log file into multiple files, by domain field.\n"
				"usage: %s <nginx_log_file> <output_folder>\n"
				"  <nginx_log_file>     nginx log file\n"
				"  <output_folder>      folder name where splitted files will save to\n"
				, __FUNCTION__);
		return 1;
	}
	FILE * f = fopen(argv[1], "r");
	if(!f) {
		fprintf(stderr, "fopen file %s failed\n", argv[1]);
		return 1;
	}
	char out_folder[32] = "logs";
	strcpy(out_folder, argv[2]);
	if(out_folder[strlen(out_folder) -1 ] != '/')
		strcat(out_folder, "/");

	std::unordered_map<std::string, FILE *> dmap;/*domain : log_file*/
	long linecount = 0;
	char data[8192] = "";
	char const * result = 0;
	while((result = fgets(data, sizeof(data), f)) != NULL){
		++linecount;
		fprintf(stdout, "\rprocessing %8ld line ...", linecount);
		int len = strlen(result);
		if(result[len - 1] != '\n'){
			fprintf(stderr, "\n%s: WARNING, length > %d bytes, skip:\n%s\n", __FUNCTION__, sizeof(data), data);
			continue;
		}
		char domain[128] = "", out_file[512] = "";
		strncpy(domain, data, strchr(data, ' ') - data);
		strlwr(domain);

		FILE * & file = dmap[domain];
		sprintf(out_file, "%s%s", out_folder, domain);
		if(!file && (file = fopen(out_file, "w")) == NULL){
			fprintf(stderr, "\n%s: WARNING, create file %s failed, skip\n", __FUNCTION__, domain);
			continue;
		}
		int result = fwrite(data, sizeof(char), len, file);
		if(result < len || ferror(file)){
			fprintf(stderr, "\n%s: WARNING, write domain file %s NOT complete:\n%s\n", __FUNCTION__, domain, data);
		}
	}
	fprintf(stdout, "\n");
	return 0;
}

static int load_sitelist(char const* file, std::unordered_map<std::string, site_info>& sitelist)
{
	int ret = -1;
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

static char const * str_find(char const *str, int len)
{
	if(!str || str[0] == '\0')
		return NULL;
	static std::unordered_map<std::string, char *> urls;
	len = (len != -1? len : strlen(str));
	char const * md5str = md5sum(str, len);
	char *& s = urls[md5str];
	if(!s){
		s = new char[len + 1];
		strncpy(s, str, len);
		s[len] = '\0';
	}
	return s;
}
static int do_test(int argc, char ** argv)
{
	//	test_strptime_main(argc, argv);
	//	test_time_mark_main(argc, argv);

		/*siteuidlist.txt www.haipin.com*/
	//	fprintf(stdout, "%s: file=%s, site=%s, id=%d\n", __FUNCTION__, argv[1], argv[2], find_site_id(argv[1], argv[2]));

	//	test_get_if_addrs_main(argc, argv);
	//	test_alloc_mmap_main(argc, argv);
	//	fprintf(stdout, "done, press anykey to exit.");
	//	getc(stdin);

	//	char str1[] = "hello", str2[] = "jack", str3[] =  "hello";
	//	fprintf(stdout, "%s: test str_find()\n"
	//			"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n"
	//			"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n", __FUNCTION__,
	//			str1, str1, str_find(str1), str2, str2, str_find(str2),
	//			str3, str3, str_find(str3), str2, str2, str_find(str2));

//	char str1[] = "hello", str2[] = "HELLO";
//	fprintf(stdout, "strlupr(): str=%s, strupr=%s\n, strlwr(): str=%s, strulwr=%s\n",
//			"hello", strupr(str1), "HELLO", strlwr(str2));
	int core_num = get_nprocs();
	fprintf(stdout, "%s: core_num=%d\n", __FUNCTION__, core_num);
	return 0;
}

int parse_log_item_buf(char const * buf, size_t len, std::map<time_mark, log_stat> & logstats, std::vector<long> & failed_lines)
{
	long linecount = 0;
	time_mark m;
	for(char const * p = buf, * q = p; q != buf + len; ++q){
		if(*q != '\n') continue;
		++linecount;
		if(linecount % 10000 == 0){
			long total = 0;
			pthread_mutex_lock(&g_line_count_mutex);
			g_line_count += linecount;
			total = g_line_count;
			pthread_mutex_unlock(&g_line_count_mutex);
			linecount = 0;

			pthread_mutex_lock(&g_io_mutex);
			fprintf(stdout, "\r%s: processing %-8ld line ...", __FUNCTION__, total);
			fflush(stdout);
			pthread_mutex_unlock(&g_io_mutex);
		}
		log_item item;
//		int result = parse_log_item(item, p, '\n');
//		p = q + 1;
//		if(result != 0){
//			if(failed_lines.size() < 10)
//				failed_lines.push_back(linecount);
//			continue;
//		}
//		log_stats(m, item, logstats);
	}
	pthread_mutex_lock(&g_line_count_mutex);
	g_line_count += linecount;
	pthread_mutex_unlock(&g_line_count_mutex);
	return 0;
}

void * parallel_parse_thread_func(void * varg)
{
	parallel_parse_thread_arg * arg = (parallel_parse_thread_arg*)varg;
	parse_log_item_buf(arg->buf, arg->len, arg->logstats, arg->failed_lines);
	return varg;
}

int parallel_parse(FILE * f, std::map<time_mark, log_stat> & stats, std::vector<long> & failedlines)
{
	struct stat logfile_stat;
	if(fstat(fileno(f), &logfile_stat) < 0){
		fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "nginx_log_file");
		return 1;
	}
	int para_count = get_nprocs();			/*mew parallel count, maybe 0*/
	size_t min_bytes = 1024 * 1024 * 128; 	/*min 64MB*/
	for(size_t c = logfile_stat.st_size / min_bytes; c < (size_t)para_count; --para_count)
	{ /*empty*/ }
	fprintf(stdout, "%s: para_count=%d\n", __FUNCTION__, para_count);

	std::vector<pthread_t> threads;

	off_t offset_p = 0;
	void * start_p = mmap(NULL, logfile_stat.st_size, PROT_READ, MAP_SHARED, fileno(f), 0);
	if(!start_p || start_p == MAP_FAILED){
		fprintf(stderr, "%s: mmap() failed for %s\n", __FUNCTION__, "nginx_log_file");
		return 1;
	}
	for(int i = 0; i < para_count; ++i){
		static size_t len = logfile_stat.st_size / para_count;

		pthread_t tid;
		auto * arg = new parallel_parse_thread_arg;
		arg->buf = (char const *)start_p + offset_p;
		arg->len = len;
		int result = pthread_create(&tid, NULL, parallel_parse_thread_func, arg);
		if(result != 0){
			fprintf(stderr, "%s: pthread_create() failed, parallel index=%d\n", __FUNCTION__, i);
			return 1;
		}
		threads.push_back(tid);
		offset_p += len;
	}
	size_t left_len = logfile_stat.st_size - offset_p;
	parallel_parse_thread_arg arg;
	arg.buf = (char const *)start_p + offset_p;
	arg.len = left_len;
	parse_log_item_buf(arg.buf, arg.len, arg.logstats, arg.failed_lines);

	void * tret;
	for(auto & item : threads){
		pthread_join(item, &tret);
//		fprintf(stdout, "%s: thread=%ld exited\n", __FUNCTION__, item);
	}
	return 0;
}

int test_nginx_log_stats_main(int argc, char ** argv)
{
	do_test(argc, argv);
	if(argc == 3 && strcmp(argv[1], "--device-id") == 0){	//query device_id and return
		int result = load_devicelist(argv[2], g_devicelist);
		if(result != 0){
			fprintf(stdout, "0\n");
			return 1;
		}
		fprintf(stdout, "%d\n", get_device_id(g_devicelist));
		return 0;
	}
	if(argc < 6){
		fprintf(stderr, "%s: argc=%d, argc >= 6 required\n",__FUNCTION__, argc);
		return 1;
	}
//	fprintf(stdout, "%s: nginx_file=%s, interval=%s, devicelist=%s, sitelist=%s, outputfile=%s\n",
//			__FUNCTION__, argv[1], argv[2], argv[3], argv[4], argv[5]);

	int result = load_devicelist(argv[3], g_devicelist);
	if(result != 0){
		fprintf(stderr, "%s: load_devicelist() failed\n", __FUNCTION__);
		return 1;
	}
	result = load_sitelist(argv[4], g_sitelist);
	if(result != 0){
		fprintf(stderr, "%s: load_sitelist() failed\n", __FUNCTION__);
		return 1;
	}
	FILE * nginx_log_file = fopen(argv[1], "r");
	if(!nginx_log_file) {
		fprintf(stderr, "fopen file %s failed\n", argv[1]);
		return 1;
	}
	FILE * output_file = fopen(argv[5], "a");
	if(!output_file) {
		fprintf(stderr, "fopen file %s for append failed\n", argv[5]);
		return 1;
	}
	int interval = atoi(argv[2]);
	if(interval < 300 || interval > 3600){
		fprintf(stdout, "%s: WARNING, interval(%d) too %s\n", __FUNCTION__, interval, interval < 300? "small" : "large");
	}
	time_mark::_interval_sec = interval;

	int device_id = 0, site_id = 0, user_id  = 0;
	device_id = get_device_id(g_devicelist);
	find_site_id(g_sitelist, find_domain(argv[1]), site_id, &user_id);
//	fprintf(stdout, "%s: device_id=%d, site_id=%d, user_id=%d\n", __FUNCTION__,
//			device_id, site_id, user_id);
	pthread_mutex_init(&g_io_mutex, NULL);
	pthread_mutex_init(&g_line_count_mutex, NULL);

	std::map<time_mark, log_stat> logstats;
	std::vector<long> failed_lines;

	parallel_parse(nginx_log_file, logstats, failed_lines);

	fprintf(stdout, "\r%s: processed, total_line: %-8.0f\n", __FUNCTION__, g_line_count);
	if(!failed_lines.empty()){
		fprintf(stdout, "%s: failed_lines:[", __FUNCTION__);
		for(size_t i = 0; i != failed_lines.size(); ++i)
			{ fprintf(stdout, "%ld%s", failed_lines[i], (i + 1 != failed_lines.size()? ", " : "]\n")); }
	}
//	result = print_stats(stdout, logstats, -1);
	print_flow_table(output_file, logstats, device_id, site_id, user_id);
	return 0;
}

