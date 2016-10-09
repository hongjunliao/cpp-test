#include "bd_test.h"
#include <time.h>	/*time_t, strptime*/
#include <stdio.h>
#include <string.h> /*strncpy*/
#include <vector>
#include <map>
#include <math.h> /*pow*/
#include <assert.h>

struct log_item;
class time_mark;
class log_stat;

static int test_strptime_main(int argc, char ** argv);
static int test_time_mark_main(int argc, char ** argv);

static int parse_log_item(char const * logitem, log_item & item);
static int parse_data(char const * data, std::vector<log_item> & logvec);
static int log_stats(std::vector<log_item> const& logvec, std::map<time_mark, log_stat>& logstats);
static void print_stats(FILE * stream, std::map<time_mark, log_stat> const& stats, int top = 10);
//////////////////////////////////////////////////////////////////////////////////
struct log_item{
	char time_local[21];
	char server_name[64];
	char remote_addr[16];
	char request_url[1024];
	double bytes_sent;
};

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
	char const * c_str() const;
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

const char* time_mark::c_str() const
{
//	return ctime(&_t);
	static char buff[20] = "";
	strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&_t));
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
	long n = difftime(t, _t) / _interval_sec;

	_t += n * _interval_sec;
//	fprintf(stdout, "%s: %s, t=%ld,_t=%ld\n", __FUNCTION__, ctime(&t), t, _t);

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
class log_stat{
public:
	std::map<std::string, int> _request_url;
	double _bytes_sent;
public:
	log_stat();
};

log_stat::log_stat()
: _bytes_sent(0)
{
	//none
}
/*
 * parse ' ' splitted logitem, nginx log:
 * flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] \
 * "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" \
 * "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E
 */
int do_parse_log_item(char const * szitem, int * count, char *** items)
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
				if(!(*(q + 1) == ' ' || *(q + 1) == '\0')){
					fprintf(stderr, "%s: parse error at %s\n", __FUNCTION__, q);
					goto error_return;
				}
				++q;
				char * arg = new char[q - p];
				strncpy(arg, p, q - p - 1);
				arg[q - p - 1] = '\0';
				s_items[c++] = arg;
				if(!*q)
					break;
				p = q + 1;
			}
			continue;
		}
		if(arg_start && !*q){
			fprintf(stderr, "%s: parse error\n", __FUNCTION__);
			goto error_return;
		}
		if(!arg_start && (*q == ' ' || !*q)){
			char * arg = new char[q - p + 1];
			strncpy(arg, p, q - p);
			arg[q - p] = '\0';
			s_items[c++] = arg;
			if(!*q)
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

static int parse_log_item(char const * logitem, log_item & item)
{
	memset(&item, 0, sizeof(log_item));
	int count;
	static char ** items;
	int result = do_parse_log_item(logitem, &count, &items);
	if(result != 0){
		return 1;
	}
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	strncpy(item.time_local, items[4] + 1, 20);
	/*url可能 带参数, 去掉参数*/
	char const * url = strchr(items[6], '/'); /*"GET /abc?param1=abc"*/
	char const * url_param = strchr(url, '?');
	strncpy(item.request_url, url, (url_param? url_param - url : strchr(url, ' ') - url));
	char const * p = items[8];
	char * end;
	item.bytes_sent = strtod(p, &end);
	return 0;
}

static int parse_data(char const * data, std::vector<log_item> & logvec)
{
	log_item  item;
	int result = parse_log_item(data, item);
	if(result != 0) return 1;
	logvec.push_back(item);
	return 0;
}

static void print_stats(FILE* stream, const std::map<time_mark, log_stat>& stats, int top)
{
	fprintf(stream, "%-21s%-120s%-6s%-20s\n", "Time", "Url", "Count", "Bytes");
	double bytes_total = 0;
	int access_total = 0;
	for(auto it = stats.begin(); it != stats.end(); ++it){
		log_stat const& stat = it->second;
		fprintf(stream, "%-21s%-120s%-6s%-20.0f\n", it->first.c_str(), " ", " ", stat._bytes_sent);
		bytes_total += stat._bytes_sent;
		if(top > 0 && --top <= 0)
			break;
		for(auto it2 = stat._request_url.begin(); it2 != stat._request_url.end(); ++it2){
			fprintf(stream, "%-21s%-120s%-6d%-20s\n", " ", it2->first.c_str(), it2->second, " ");
			access_total += it2->second;
		}
	}
	fprintf(stream, "Sum\n%-21s%-120s%-6d%-20.6f\n", " ", " ", access_total, bytes_total / pow(1024.0, 3));
}

inline int log_stats(const std::vector<log_item>& logvec,
		std::map<time_mark, log_stat>& logstats)
{
	time_mark m;
	for(log_item const& item : logvec){
		m.mark(item.time_local);
		log_stat& logsstat = logstats[m];
		++logsstat._request_url[item.request_url];
		logsstat._bytes_sent += item.bytes_sent;
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

static int test_nginx_log_split_main(int argc, char ** argv)
{
	if(argc < 3){
		fprintf(stderr, "split nginx log file into multiple files, by domain.\n"
				"usage: %s <nginx_log_file> <output_folder>\n"
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

	std::map<std::string, FILE *> dmap;/*domain : log_file*/
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

static int test_nignx_log_stats_main(int argc, char ** argv)
{
	char data[1024] = "";
	FILE * f = fopen(argv[1], "r");
	if(!f) {
		fprintf(stderr, "fopen file %s failed\n", argv[1]);
		return 1;
	}
	std::vector<log_item> logs;
	long linecount = 0;
	while(fgets(data, sizeof(data), f)){
		++linecount;
		fprintf(stdout, "\rprocessing %8ld line ...", linecount);
		fflush(stdout);
		log_item item;
		int result = parse_log_item(data, item);
		if(result != 0){
			fprintf(stderr, "parse failed for %s\n", data);
			continue;
		}
		logs.push_back(item);
	}
	fprintf(stdout, "\n");
	time_mark::_interval_sec = 3600;
	std::map<time_mark, log_stat> logstats;
	log_stats(logs, logstats);
	print_stats(stdout, logstats, -1);
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////
int test_nginx_log_analysis_main(int argc, char ** argv)
{
//	test_strptime_main(argc, argv);
//	test_time_mark_main(argc, argv);

	test_nginx_log_split_main(argc, argv);
//	test_nignx_log_stats_main(argc, argv);

	return 0;
}

