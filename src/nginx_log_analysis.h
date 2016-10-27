/*!
 * This file is PART of nginx_log_analysis
 * data structs
 */
#ifndef _NGINX_LOG_ANALYSIS_H_
#define _NGINX_LOG_ANALYSIS_H_
#include <time.h>	/*time_t, strptime*/
#include <unordered_map> /*std::unordered_map*/
#include <map>	/*std::map*/
#include <string> /*std::string*/
/*declares*/
struct log_item;
struct site_info;
struct parse_context;
struct url_count;
class time_group;
class url_stat;
struct ip_stat;
class log_stat;
struct locisp_stat;
class locisp_group;
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
	std::string/*char const **/ url;
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
	std::map<time_group, log_stat> logstats;
	size_t total_lines;
};

//////////////////////////////////////////////////////////////////////////////////
/*use time interval for group nginx logs*/
class time_group
{
public:
	static int _sec;	/*in seconds*/
private:
	time_t _t;
public:
	explicit time_group(char const * strtime = NULL);
public:
	operator bool() const;
	time_group next() const;
	time_group& group(char const * strtime);
	time_group& group(time_t const& t);
	/*use lcoal static buffer, NOT thread-safe*/
	char const * c_str(char const * fmt = "%Y-%m-%d %H:%M:%S") const;
	/*thread-safe version*/
	char * c_str_r(char * buff, size_t len, char const * fmt = "%Y-%m-%d %H:%M:%S") const;
private:
	friend bool operator ==(const time_group& one, const time_group& another);
	friend bool operator <(const time_group& one, const time_group& another);
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
struct locisp_stat
{
	size_t bytes; 	/*bytes total*/
	size_t access;	/*access count*/
	size_t bytes_m;			/*bytes total for nginx 'MISS' */
	size_t access_m;		/*access_count for nginx 'MISS'*/
};

//////////////////////////////////////////////////////////////////////////////////
/*group by local_id and isp: locisp*/
class locisp_group
{
	char _locisp[8 + 1];
	friend bool operator ==(const locisp_group& one, const locisp_group& another);
	friend struct std::hash<locisp_group>;
public:
	locisp_group(char const * data= "");
};
bool operator ==(const locisp_group& one, const locisp_group& another);

//////////////////////////////////////////////////////////////////////////////////
//@reference: http://www.cppreference.com/ ?
namespace std{
template<> struct hash<locisp_group>
{
	typedef locisp_group argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};
}	//namespace std

//////////////////////////////////////////////////////////////////////////////////
/*!
 * log statistics
 * FIXME: bytes_total1 = sum(_ip_stats), bytes_total2 = sum(_url_stats)
 * make sure bytes_total1 == bytes_total2
 * */
class log_stat
{
public:
	/*!
	 * FIXME: @see str_find NOT correct in multi-thread, and little speedup
	 * becuase of this, change _url_stats<char *, url_stat> back to _url_stats<std::string, url_stat>, @date 2016/1027
	 */
	std::unordered_map<std::string/*char const **/, url_stat> _url_stats;	/*url:url_stat*/
	std::unordered_map<uint32_t, ip_stat> _ip_stats;						/*ip:ip_stat*/
//	std::unordered_map<locisp_group, locisp_stat> _lid_isp_stats;			/*ip:ip_stat*/
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

