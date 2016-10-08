#include "bd_test.h"
#include <time.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <assert.h>

struct log_item;
class time_mark;
class log_stat;

static int test_strptime_main(int argc, char ** argv);
static int test_time_mark_main(int argc, char ** argv);

static int parse_log_item(char const * logitem, log_item & item);
static int parse_data(char const * data, std::vector<log_item> & logvec);
static int log_stats(std::vector<log_item> const& logvec, std::map<time_mark, log_stat>& logstats);

//////////////////////////////////////////////////////////////////////////////////
struct log_item{
	char time_local[32];
	char server_name[64];
	char remote_addr[16];
	char request_url[1024];
	long bytes_sent;
};

//////////////////////////////////////////////////////////////////////////////////
class time_mark{
public:
	int _interval_sec;	/*in seconds*/
private:
	bool _is_ok;
	time_t _t;
public:
	time_mark(char const * strtime = NULL);
public:
	operator bool() const;
	time_mark next_mark() const;
	time_mark& mark(char const * strtime);
	char const * c_str() const;
private:
	friend bool operator ==(const time_mark& one, const time_mark& another);
	friend bool operator <(const time_mark& one, const time_mark& another);
};

time_mark::time_mark(char const * strtime)
: _interval_sec(300)
, _is_ok(true)
, _t(0)
{
	tm my_tm;
	if(strtime){
		char const * result = strptime(strtime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
		_is_ok = (result);
		my_tm.tm_isdst = 0;
		mktime(&my_tm);
		if(!_is_ok) return;
	}
	else{
		time_t t = time(NULL);
		my_tm = *localtime(&t);
	}
	my_tm.tm_sec = 0;
	_t = mktime(&my_tm);
}

time_mark::operator bool() const
{
	return _is_ok;
}

const char* time_mark::c_str() const
{
	return ctime(&_t);
}

time_mark& time_mark::mark(const char* strtime)
{
	if(!strtime) return *this;
	tm my_tm;
	char const * result = strptime(strtime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	if(!result) return *this;

	my_tm.tm_isdst = 0;
	time_t t = mktime(&my_tm);
	int n = (long)difftime(_t, t) / _interval_sec;

	if(_t < t)
		_t += n * _interval_sec;
	else
		_t -= n * _interval_sec;

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
	long _bytes_sent;
};

//////////////////////////////////////////////////////////////////////////////////
int test_nginx_log_analysis_main(int argc, char ** argv)
{
	test_strptime_main(argc, argv);
	test_time_mark_main(argc, argv);

	//flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E
	char const *data = "";
	std::vector<log_item> logs;
	int status = parse_data(data, logs);
	std::map<time_mark, log_stat> logstats;
	log_stats(logs, logstats);
	return 0;
}

static int parse_log_item(char const * logitem, log_item & item)
{
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

static int test_time_mark_main(int argc, char ** argv)
{
	char const * stime = "17/Sep/2016:01:19:43";
	time_t time1 = time(NULL);
	tm time2 = *localtime(&time1);
	time2.tm_hour -= 1;
	assert((time1 - 3600) == mktime(&time2));

	time_mark timem1(stime), timem2(stime);
	assert(timem1);
	assert(timem1 == timem2);
	assert(timem1 == time_mark("17/Sep/2016:01:19:00"));
	fprintf(stdout, "time=%s, start=%s\n", stime, timem1.c_str());
	for(int i= 0; i < 5; ++i){
		fprintf(stdout, "next=%s\n", timem1.next_mark().c_str());
		timem1 = timem1.next_mark();
	}
	time_mark startm("17/Sep/2016:01:19:43"), timem3(startm);
	assert(startm == timem3.mark("17/Sep/2016:01:23:43"));
	assert(startm == timem3.mark("17/Sep/2016:01:15:43"));
	return 0;
}


inline int log_stats(const std::vector<log_item>& logvec,
		std::map<time_mark, log_stat>& logstats)
{
	for(log_item const& item : logvec){
		time_mark timem(item.time_local);
		log_stat& logsstat = logstats[timem];
		++logsstat._request_url[item.request_url];
		logsstat._bytes_sent += item.bytes_sent;
	}
	return 0;
}
