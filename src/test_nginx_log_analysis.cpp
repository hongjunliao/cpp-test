/*!
 * this module parse nginx logs and print results
 * @note: 1.As of current design, the input nginx log file MUST have ONLY one domain, @see test_nginx_log_split_main
 */
#include <fnmatch.h>	/*fnmatch*/
#include <sys/sysinfo.h>	/*get_nprocs*/
#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
#include <time.h>	/*time_t, strptime*/
#include <stdio.h>
#include <string.h> /*strncpy*/
#include <math.h> /*pow*/
//#include <locale.h> /*setlocale*/
#include <assert.h>
#include <pthread.h> /*pthread*/

#include <vector>
#include <algorithm> /*std::sort*/
#include <unordered_map> /*unordered_map*/
#include <numeric>	/*std::accumulate*/
#include <map>
#include <bitset>
#include "bd_test.h"
#include "test_options.h"	/*nla_options**/

#include "termio_util.h"	/*termio_**/

/*declares*/
struct log_item;
struct site_info;
struct parse_context;
struct url_count;
class time_interval;
class url_stat;
class log_stat;

/*tests, @see do_test*/
static int test_strptime_main(int argc, char ** argv);
static int test_time_mark_main(int argc, char ** argv);
static int test_get_if_addrs_main(int argc, char ** argv);
int test_nginx_log_split_main(int argc, char ** argv);
/*test_mem.cpp*/
extern int test_alloc_mmap_main(int argc, char * argv[]);

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
 * if @param str NOT exist, then add as new, else return exist*/
static char const * str_find(char const *str, int len = -1);

/*string_util.cpp*/
extern char const * md5sum(char const * str, int len);
extern char const * byte_to_mb_kb_str(size_t bytes, char const * fmt);
/*net_util.cpp*/
extern int get_if_addrs(char *ips, int & count, int sz);

/*!
 * ouput results
 */
/*flow table*/
static void print_flow_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot url*/
static void print_url_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*hot ip*/
static void print_ip_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);
/*httpstatus_statistics*/
static void print_httpstatus_stats_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id);

static int print_stats(FILE * stream, std::map<time_interval, log_stat> const& stats, int top = 10);
//////////////////////////////////////////////////////////////////////////////////
struct log_item{
	time_t time_local;
	char const *request_url;
	size_t bytes_sent;
	int status;
	bool is_hit;
};
//////////////////////////////////////////////////////////////////////////////////
struct site_info{
	int site_id;
	int user_id;
};

//////////////////////////////////////////////////////////////////////////////////
struct url_count
{
	char const * url;
	size_t count;
};
//////////////////////////////////////////////////////////////////////////////////
/*log buffer and parse result*/
struct parse_context
{
/*input*/
	char * buf;
	size_t len;

/*output*/
	std::map<time_interval, log_stat> logstats;
	size_t total_lines;
};

/*GLOBAL vars*/
/*all options: test_options.cpp*/
extern struct nla_options nla_opt;

/*map<device_id, ip_addr>*/
static std::unordered_map<int, char[16]> g_devicelist;
static std::unordered_map<std::string, site_info> g_sitelist;
static size_t g_line_count = 0;
static pthread_mutex_t g_io_mutex;
static pthread_mutex_t g_line_count_mutex;

#define PARALLEL_FPRINTF(file, fmt, ...) \
	{pthread_mutex_lock(&g_io_mutex); \
	fprintf(file, fmt, __VA_ARGS__); \
	pthread_mutex_unlock(&g_io_mutex); \
	}
//////////////////////////////////////////////////////////////////////////////////
class time_interval
{
public:
	static int _sec;	/*in seconds*/
private:
	time_t _t;
public:
	explicit time_interval(char const * strtime = NULL);
public:
	operator bool() const;
	time_interval next_mark() const;
	time_interval& mark(char const * strtime);
	time_interval& mark(time_t const& t);
	char const * c_str(char const * fmt = "%Y-%m-%d %H:%M:%S") const;
private:
	friend bool operator ==(const time_interval& one, const time_interval& another);
	friend bool operator <(const time_interval& one, const time_interval& another);
};

int time_interval::_sec = 300;
time_interval::time_interval(char const * strtime)
: _t(0)
{
	if(strtime)
		mark(strtime);
}

time_interval::operator bool() const
{
	return 1;
}

const char* time_interval::c_str(char const * fmt) const
{
//	return ctime(&_t);
	/*FIXME: thread-safe*/
	static char buff[20] = "";
	if(!fmt)
		return "<null time_mark>";
	strftime(buff, 20, fmt, localtime(&_t));
	return buff;
}

time_interval& time_interval::mark(const char* strtime)
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

time_interval& time_interval::mark(time_t const& t)
{
	long n = difftime(t, _t) / _sec;
	_t += n * _sec;
	return *this;
}

bool operator <(const time_interval& one, const time_interval& another)
{
	return one._t < another._t;
}

bool operator ==(const time_interval& one, const time_interval& another)
{
	return one._t == another._t;
}

time_interval time_interval::next_mark() const
{
	time_interval ret(*this);
	ret._t += _sec;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////
/*url statistics*/
class url_stat
{
public:
	std::unordered_map<int, size_t> _status;		/*http_status_code: access_count*/
	std::unordered_map<int, size_t> _bytes;	/*http_status_code: bytes*/
public:
	size_t access_total() const;
	size_t bytes(int code1, int code2 = -1) const;
	size_t bytes_total() const;
	size_t access(int code1, int code2 = -1) const;
public:
	url_stat& operator+=(url_stat const& another);
};

size_t url_stat::access_total() const
{
	size_t ret  = 0;
	for(auto it = _status.begin(); it !=_status.end(); ++it){
		ret += it->second;
	}
	return ret;
}

size_t url_stat::access(int code1, int code2/* = -1*/) const
{
	size_t c1 = (_status.count(code1) != 0? _status.at(code1) : 0);
	size_t c2 = (_status.count(code2) != 0? _status.at(code2) : 0);
	return c1 + c2;
}

size_t url_stat::bytes(int code1, int code2/* = -1*/) const
{
	size_t ret  = 0;
	for(auto it = _bytes.begin(); it !=_bytes.end(); ++it){
		if(it->first == code1 || it->first == code2)
			ret += it->second;
	}
	return ret;

}

size_t url_stat::bytes_total() const
{
	size_t ret  = 0;
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
/*url statistics in time interval*/
class log_stat
{
public:
	std::unordered_map<char const *, url_stat> _url_stats;	/*url:url_stat*/
	size_t _bytes_m;	/*bytes for "CDN back to source"*/
	size_t _access_m;		/*access_count for "CDN back to source"*/
public:
	log_stat();
public:
	size_t bytes_total() const;
	size_t bytes(int code1, int code2 = -1) const;
	size_t access_total() const;
	size_t access(int code1, int code2 = -1) const;
	log_stat& operator+=(log_stat const& another);
};

log_stat::log_stat()
: _bytes_m(0)
, _access_m(0)
{
	//none
}

size_t log_stat::access_total() const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access_total();
	}
	return ret;
}

inline size_t log_stat::bytes(int code1, int code2/* = -1*/) const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes(code1, code2);
	}
	return ret;
}

inline size_t log_stat::bytes_total() const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes_total();
	}
	return ret;
}

size_t log_stat::access(int code1, int code2/* = -1*/) const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access(code1, code2);
	}
	return ret;
}

log_stat& log_stat::operator+=(log_stat const& another)
{
	for(auto const& item : another._url_stats){
		_url_stats[item.first] += item.second;
	}
	_access_m += another._access_m;
	_bytes_m += another._bytes_m;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////

static int parse_log_item(log_item & item, char *& logitem, char delim /*= '\0'*/)
{
	memset(&item, 0, sizeof(log_item));
	char *items[18];
	int result = do_parse_log_item(items, logitem, delim);
	if(result != 0){
		return 1;
	}
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	tm my_tm;
	if(!strptime(items[4] + 1, "%d/%b/%Y:%H:%M:%S" , &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	item.time_local = mktime(&my_tm);
	char * url = strchr(items[6], '/'); /*"GET /abc?param1=abc"*/
	int len = strchr(url, ' ') - url;
	url[len] = '\0';
	item.request_url = str_find(url, len);

	char const * p = items[8];
	char * end;
	item.bytes_sent = strtoul(p, &end, 10);
	item.status = atoi(items[7]);
	item.is_hit = (strcmp(strupr(items[3]),"HIT") == 0);
	return 0;
}

bool compare_by_access_count(url_count const& a, url_count const& b)
{
	return a.count > b.count;
}
static void url_top_n(std::map<time_interval, log_stat> const& stats, std::vector<url_count>& urlcount)
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

static int print_stats(FILE* stream, const std::map<time_interval, log_stat>& stats, int top)
{
	fprintf(stream, "%-20s%-120s%-10s%-20s\n", "Time", "Url", "Count", "Bytes");
	size_t bytes_total = 0, bytes_200 = 0, bytes_206 = 0, other_bytes = 0;
	size_t access_total = 0, access_200 = 0, access_206 = 0;
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
	fprintf(stream, "%-15ld%-15ld%-20ld%-20zu%-20zu%-20zu%-20zu%-20zu/%-20.6f\n",
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

static void print_flow_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		log_stat const& stat = item.second;
		char line[512];
		/*format: site_id, datetime, device_id, num_total, bytes_total, user_id, pvs_m, px_m */
		int sz = snprintf(line, 512, "%d %s %d %ld %zu %d %ld %zu\n",
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

inline void print_url_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		log_stat const& stat = item.second;
		for(auto const& url_item : stat._url_stats){
			auto const& url = url_item.first;
			auto const& st = url_item.second;
			size_t num_total = st.access_total(), num_200 = st.access(200), size_200 = st.bytes(200)
					, num_206 = st.access(206), size_206 = st.bytes(206), num_301302 = st.access(301, 302)
					, num_304 = st.access(304) , num_403 = st.access(403), num_404 = st.access(404), num_416 = st.access(416)
					, num_499 = st.access(499), num_500 = st.access(500), num_502 = st.access(502)
					, num_other = num_total - (num_200 + num_206 + num_301302 + num_304 + num_403
									+ num_404 + num_416 + num_499 + num_500 + num_502)
					;
			char line[512];
			/*format:
			 *datetime, url_key, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
			 *, num_403, num_404, num_416, num_499, num_500, num_502, num_other*/
			/*FIXME: md5sum(url)?*/
			int sz = snprintf(line, 512, "%s %s %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",
						item.first.c_str("%Y%m%d%H%M"), url
						, num_total, num_200, size_200, num_206, size_206, num_301302, num_304
						, num_403, num_404, num_416, num_499, num_500, num_502
						, num_other
						);
			if(sz <= 0){
				++i;
				continue;
			}
			buff += line;
			if(buff.size() > 1024 * 1024 * 16){
				size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
				if(r != buff.size())
//					fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n", __FUNCTION__, buff.size(), r);
				buff.clear();
			}
		}
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
//		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n", __FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

inline void print_ip_popular_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{

}

inline void print_httpstatus_stats_table(FILE * stream, std::map<time_interval, log_stat> const& stats,
		int device_id, int site_id, int user_id)
{
	std::string buff;
	int i = 0;
	for(auto const& item : stats){
		std::unordered_map<int, size_t> st;	/*http_status_code: access_count*/
		for(auto const& url_item : item.second._url_stats){
			for(auto const& httpcode_item : url_item.second._status){
				st[httpcode_item.first] += httpcode_item.second;
			}
		}
		char line[512];
		for(auto const& st_item : st){
			/*format: site_id, device_id, httpstatus, datetime, num*/
			int sz = snprintf(line, 512, "%d %d %d %s %zu\n",
					site_id, device_id, st_item.first, item.first.c_str("%Y%m%d%H%M"), st_item.second);
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
	}
	size_t r = fwrite(buff.c_str(), sizeof(char), buff.size(), stream);
	if(r != buff.size())
		fprintf(stderr, "%s: WARNING, total=%ld, written=%ld\n",
				__FUNCTION__, buff.size(), r);

	if(i != 0)
		fprintf(stderr, "%s: WARNING, skip %d lines\n", __FUNCTION__, i);
}

static int log_stats(time_interval & m, log_item const& item, std::map<time_interval, log_stat>& logstats)
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
				PARALLEL_FPRINTF(stdout, "%s: total=%s\n", __FUNCTION__, byte_to_mb_kb_str(total, "%-.0f %cB"));
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
	std::string md5str = md5sum(str, len);
	auto &s = urls[md5str];
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

int parallel_parse(FILE * f, std::map<time_interval, log_stat> & stats)
{
	struct stat logfile_stat;
	if(fstat(fileno(f), &logfile_stat) < 0){
		fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "nginx_log_file");
		return 1;
	}
	int parallel_count = get_nprocs() - 1;			/*parallel count, maybe 0*/
	size_t min_bytes = 1024 * 1024 * 128; 	/*min 128MB*/
	for(size_t c = logfile_stat.st_size / min_bytes; c < (size_t)parallel_count; --parallel_count){ /*empty*/ }
	/*!
	 * FIXME:
	 * 1. this function(parallel_parse) currently NOT thread-safe  because of static vars defined in other functions it called
	 * set parallel count to 0 yet, @datetime: 2016/10/21
	 * */
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
	parse_context parse_args[parallel_count + 1];
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
		threads[i] = tid;
		offset_p += nlen;
	}
	size_t left_len = logfile_stat.st_size - offset_p;
	if(nla_opt.verbose){
		printf("%s: main_thread process size=%zu, percent=%.1f%%\n", __FUNCTION__, left_len, (double)left_len * 100/ logfile_stat.st_size);
	}
	auto & arg = parse_args[parallel_count];
	arg.buf = start_p + offset_p;
	arg.len = left_len;
	arg.total_lines = 0;
	parse_log_item_buf(arg);

	for(auto & item : threads){
		void * tret;
		pthread_join(item, &tret);
		fprintf(stdout, "%s: thread=%zu exited\n", __FUNCTION__, item);
	}
	if(parallel_count == 0){
		/*FIXME: NOT needed?*/
		stats.swap(parse_args[0].logstats);
		g_line_count = parse_args[0].total_lines;
	}
	else{
		for(auto const& item : parse_args){
			log_stats_append(stats, item.logstats);
			g_line_count += item.total_lines;
		}
	}
	munmap(start_p, logfile_stat.st_size);
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

	time_interval timem1(stime), timem2(stime);
	assert(timem1);
	assert(timem1 == timem2);
	assert(timem1 == time_interval("17/Sep/2016:01:19:00"));
	fprintf(stdout, "time=17/Sep/2016:01:19:23,mark=%s\n", time_interval().mark("17/Sep/2016:01:19:23").c_str());
	fprintf(stdout, "time=%s, start=%s\n", stime, timem1.c_str());
	for(int i= 0; i < 5; ++i){
		fprintf(stdout, "next=%s\n", timem1.next_mark().c_str());
		timem1 = timem1.next_mark();
	}
	time_interval startm("17/Sep/2016:01:19:43"), timem3(startm);
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
	size_t linecount = 0;
	char data[8192] = "";
	char const * result = 0;
	while((result = fgets(data, sizeof(data), f)) != NULL){
		++linecount;
		fprintf(stdout, "\rprocessing %8ld line ...", linecount);
		int len = strlen(result);
		if(result[len - 1] != '\n'){
			fprintf(stderr, "\n%s: WARNING, length > %zu bytes, skip:\n%s\n", __FUNCTION__, sizeof(data), data);
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

//		char str1[] = "hello", str2[] = "jack", str3[] =  "hello";
//		fprintf(stdout, "%s: test str_find()\n"
//				"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n"
//				"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n", __FUNCTION__,
//				str1, str1, str_find(str1), str2, str2, str_find(str2),
//				str3, str3, str_find(str3), str2, str2, str_find(str2));

//	char str1[] = "hello", str2[] = "HELLO";
//	fprintf(stdout, "strlupr(): str=%s, strupr=%s\n, strlwr(): str=%s, strulwr=%s\n",
//			"hello", strupr(str1), "HELLO", strlwr(str2));
//	int core_num = get_nprocs();
//	fprintf(stdout, "%s: core_num=%d\n", __FUNCTION__, core_num);
//	int term_cols = 0, term_rows = 0;
//	termio_get_col_row(&term_cols, &term_rows);
//	fprintf(stdout, "term_cols = %d, term_rows = %d\n", term_cols, term_rows);
	return 0;
}

int test_nginx_log_stats_main(int argc, char ** argv)
{
	do_test(argc, argv);
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
	FILE * output_file = fopen(nla_opt.output_file, "a");
	if(!output_file) {
		fprintf(stderr, "fopen file %s for append failed\n", nla_opt.output_file);
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
	pthread_mutex_init(&g_io_mutex, NULL);
	pthread_mutex_init(&g_line_count_mutex, NULL);

	std::map<time_interval, log_stat> logstats;
	parallel_parse(nginx_log_file, logstats);
	if(nla_opt.verbose)
		fprintf(stdout, "\r%s: processed, total_line: %-8ld\n", __FUNCTION__, g_line_count);
//	result = print_stats(stdout, logstats, -1);
	print_flow_table(output_file, logstats, device_id, site_id, user_id);

//	print_url_popular_table(stdout, logstats, device_id, site_id, user_id);
//	print_ip_popular_table(stdout, logstats, device_id, site_id, user_id);
//	print_httpstatus_stats_table(stdout, logstats, device_id, site_id, user_id);
	str_find(" ", -2);
	return 0;
}
