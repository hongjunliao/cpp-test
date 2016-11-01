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
class cutip_group;
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
	static int _sec;	/*interval in seconds*/
private:
	time_t _t;
public:
	/*can be implicit construct*/
	time_group(char const * strtime = NULL);
	time_group(time_t const& t);
public:
	time_group next() const;
	/*use lcoal static buffer, NOT thread-safe*/
	char const * c_str(char const * fmt = "%Y-%m-%d %H:%M:%S") const;
	/*thread-safe version*/
	char * c_str_r(char * buff, size_t len, char const * fmt = "%Y-%m-%d %H:%M:%S") const;
private:
	void group(char const * strtime);
	void group(time_t const& t);
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
/* group by local_id and isp: locisp
 * @note: define ENABLE_IPMAP to enable ipmap*/
class locisp_group
{
	char _locisp[32];
	friend bool operator ==(const locisp_group& one, const locisp_group& another);
	friend struct std::hash<locisp_group>;
public:
	locisp_group(uint32_t ip = 0);
	/*buff sample: 'CN3501 0B'*/
	void loc_isp_c_str(char * buff, int len) const;
};

bool operator ==(const locisp_group& one, const locisp_group& another);
//////////////////////////////////////////////////////////////////////////////////
/*!
 * group by first 3 fields of ip, e.g. "192.168.212.*",
 * @see print_cutip_slowfast_table
 * */
class cutip_group
{
	char _cutip[16]; /*ip with first 3 fields, include NULL, e.g. 192.168.212*/
	friend bool operator ==(const cutip_group& one, const cutip_group& another);
	friend struct std::hash<cutip_group>;
public:
	cutip_group(uint32_t ip = 0);
	/*@param buff[12] as lest, sample output: 192.168.212*/
	char const * c_str() const;
};
//////////////////////////////////////////////////////////////////////////////////
//required by std::unordered_map's key, @see http://en.cppreference.com/w/cpp/utility/hash
namespace std{
template<> struct hash<locisp_group>
{
	typedef locisp_group argument_type;
	typedef std::size_t result_type;
	result_type operator()(argument_type const& s) const;
};

template<> struct hash<cutip_group>
{
	typedef cutip_group argument_type;
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
	std::unordered_map<uint32_t, ip_stat> _ip_stats;				/*ip:ip_stat*/
	std::unordered_map<cutip_group, ip_stat> _cuitip_stats;			/*cutip: ip_stat*/
	std::unordered_map<locisp_group, locisp_stat> _locisp_stats;	/*locisp:locisp_stat*/
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

