/*!
 * This file is PART of nginx_log_analysis
 * data structs
 */
#ifndef _NGINX_LOG_ANALYSIS_H_
#define _NGINX_LOG_ANALYSIS_H_
#include <time.h>	/*time_t, strptime*/
#include <unordered_map> /*unordered_map*/
#include <map>

/*declares*/
struct log_item;
struct site_info;
struct parse_context;
struct url_count;
class time_interval;
class url_stat;
struct ip_stat;
class log_stat;

//////////////////////////////////////////////////////////////////////////////////
/*a line of nginx log*/
struct log_item{
	time_t time_local;
	char const *request_url;
	size_t request_time;
	uint32_t client_ip;
	size_t bytes_sent;
	int status;
	bool is_hit;
};
//////////////////////////////////////////////////////////////////////////////////
/*@see load_sitelist*/
struct site_info{
	int site_id;
	int user_id;
};

//////////////////////////////////////////////////////////////////////////////////
/*@see top_url_n*/
struct url_count
{
	char const * url;
	size_t count;
};
//////////////////////////////////////////////////////////////////////////////////
/*log buffer and parse result, @see parse_log_item_buf*/
struct parse_context
{
/*input*/
	char * buf;
	size_t len;

/*output*/
	std::map<time_interval, log_stat> logstats;
	size_t total_lines;
};

//////////////////////////////////////////////////////////////////////////////////
/*time interval for group nginx logs*/
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

//////////////////////////////////////////////////////////////////////////////////
/*url statistics*/
class url_stat
{
public:
	std::unordered_map<int, size_t> _status;	/*http_status_code: access_count*/
	std::unordered_map<int, size_t> _bytes;		/*http_status_code: bytes*/
public:
	size_t access_total() const;
	size_t bytes(int code1, int code2 = -1) const;
	size_t bytes_total() const;
	size_t access(int code1, int code2 = -1) const;
public:
	url_stat& operator+=(url_stat const& another);
};

//////////////////////////////////////////////////////////////////////////////////
/*client ip statistics, @see log_stat*/
struct ip_stat
{
	size_t bytes; 	/*bytes total*/
	size_t sec;		/*time total, in seconds*/
	size_t access;	/*access count*/
};
//////////////////////////////////////////////////////////////////////////////////
/*!
 * log statistics
 * FIXME: bytes_total1 = sum(_ip_stats), bytes_total2 = sum(_url_stats)
 * make sure bytes_total1 == bytes_total2
 * */
class log_stat
{
public:
	std::unordered_map<char const *, url_stat> _url_stats;	/*url:url_stat*/
	std::unordered_map<uint32_t, ip_stat> _ip_stats;		/*ip:ip_stat*/
	size_t _bytes_m;		/*bytes for nginx 'MISS' */
	size_t _access_m;		/*access_count for nginx 'MISS'*/
public:
	log_stat();
public:
	size_t bytes_total() const;
	size_t bytes(int code1, int code2 = -1) const;
	size_t access_total() const;
	size_t access(int code1, int code2 = -1) const;
	log_stat& operator+=(log_stat const& another);
};

#endif /*_NGINX_LOG_ANALYSIS_H_*/

