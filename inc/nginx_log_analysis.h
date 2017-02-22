/*!
 * This file is PART of nginx_log_analysis
 * data structs
 */
#ifndef _NGINX_LOG_ANALYSIS_H_
#define _NGINX_LOG_ANALYSIS_H_
#include <time.h>	/*time_t, strptime*/
#include <unordered_map> /*std::unordered_map*/
#include <vector>	/*std::vector*/
#include <map>	/*std::map*/
#include <string> /*std::string*/
#include "test_options.h"	/* plcdn_la_options */
/*declares*/
struct log_item;
struct site_info;
struct parse_context;
struct url_count;
class time_group;
class url_stat;
struct ip_stat;
class nginx_log_stat;
struct locisp_stat;
class locisp_group;
class cutip_group;
//////////////////////////////////////////////////////////////////////////////////
/*a line of nginx log*/
struct log_item{
	char * beg, * end;	/*raw*/
	char const * domain;
	time_t time_local;
	char const *request_url;
	size_t request_time;
	uint32_t client_ip;
//	char const * client_ip_2;	/*same as client_ip*/
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
	/*thread-safe version*/
	char * c_str_r(char * buff, size_t len, char const * fmt = "%Y%m%d%H%M") const;
	time_t t() const;
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
#ifdef ENABLE_IPMAP
#include "ipmap.h"	/*ipmap_ctx*/
#endif //ENABLE_IPMAP

class locisp_group
{
	char _locisp[32];
	friend bool operator ==(const locisp_group& one, const locisp_group& another);
	friend struct std::hash<locisp_group>;
#ifdef ENABLE_IPMAP
private:
	static struct ipmap_ctx _ipmap_ctx;
#endif //ENABLE_IPMAP
public:
	locisp_group(uint32_t ip = 0);
#ifdef ENABLE_IPMAP
public:
	static int load_ipmap_file(char const * ipmap_file);
#endif //ENABLE_IPMAP
public:
	/*buff sample: 'CN3501 0B'*/
	char * loc_isp_c_str(char * buff, int len) const;
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
	/*@param cutip, ip with first 3 fields*/
	cutip_group(char const * cutip = NULL);
public:
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
//for nginx origin log, pair<beg, end>
struct nginx_raw_log: public std::pair<char *, char *>
{
	/* a nginx_raw_log is either a std::pair<char *, char *>, which from mmap call
	 * (the 'first' and 'second' member should always be valid before munmap called)
	 * or a buffer(std::string),  which from fgets(or getline, etc) call
	 * */
	std::string buff;

	nginx_raw_log(std::pair<char *, char *> const& val);
	nginx_raw_log(std::string const& val);
	nginx_raw_log(char const* val = "");
};

typedef nginx_raw_log nginx_raw_log_t;

/*!
 * log statistics
 * FIXME: bytes_total1 = sum(_ip_stats), bytes_total2 = sum(_url_stats)
 * make sure bytes_total1 == bytes_total2
 * */
class nginx_log_stat
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
	/* http_status_code: access_count,
	 * access_count for nginx 'MISS', grouped by http_status_code*/
	std::unordered_map<int, size_t> _access_m;
	/* @date 2017/02/16 bytes from srs, @see append_flow_nginx */
	size_t srs_in, srs_out;
public:
	std::vector<nginx_raw_log_t> _logs;
public:
	nginx_log_stat();
public:
	size_t bytes_total() const;
	size_t bytes(int code1, int code2 = -1) const;
	size_t access_total() const;
	size_t access_m() const;
	size_t access(int code1, int code2 = -1) const;
	nginx_log_stat& operator+=(nginx_log_stat const& another);
public:
	/* if all members empty(except _logs), return true */
	bool empty() const;
};

//////////////////////////////////////////////////////////////////////////////////
/*log statistics by domain*/
struct nginx_domain_stat
{
	std::map<time_group, nginx_log_stat> _stats;
	int _site_id;
	int _user_id;
};

//////////////////////////////////////////////////////////////////////////////////
/*log buffer and parse result, @see parse_log_item_buf*/
struct parse_context
{
/*input*/
	char * buf;
	size_t len;

/*output*/
	std::unordered_map<std::string, nginx_domain_stat> logstats;	/*domain : domain_stat*/
	size_t total_lines;
};

//////////////////////////////////////////////////////////////////////////////////
/* load sitelist, @param file:
 * format: '<site_id> <user_id> <domain> ? ...'
 * sample:
 * '377 57 zuzhou.zqgame.com 0 119.147.215.106:80 58.254.169.106:80'
 */
int load_sitelist(char const* file, std::unordered_map<std::string, site_info>& sitelist);
/*find site_id by site_name/domain, return 0 on success*/
int find_site_id(std::unordered_map<std::string, site_info> const& sitelist,
		const char* site, int & siteid, int * user_id);

/* if @param t in [@param begin, @param end) return true */
bool is_time_in_range(time_t const& t, time_t const& begin, time_t const& end);

/*!
 * parse ' ' splitted nginx log
 * @NOTE:
 * 1.current nginx_log format:
 * $host $remote_addr $request_time_msec $cache_status [$time_local] "$request_method \
 * $request_uri $server_protocol" $status $bytes_sent \
 * "$http_referer" "$remote_user" "$http_cookie" "$http_user_agent" \
 * $scheme $request_length $upstream_response_time', total fields == 18
 *
 * nginx_log sample:
 * flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] \
 * "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" \
 * "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E

 * TODO:make it customizable
 * */
int do_parse_nginx_log_item(char ** fields, char *& szitem, char delim = '\0');

/* overloaded version, return array of  std::pair, will NOT modify @param szitem */
int do_parse_nginx_log_item(std::pair<char const *, char const *> * fields, char const * szitem, char delim = '\0');

/* parse ' ' splitted nginx log into log_item @param item
 * @return 0 on success */
int parse_log_item(log_item & item, char *& logitem, char delim = '\0', int parse_url_mode = 2);

/*do log statistics with time interval*/
int do_nginx_log_stats(log_item const& item, plcdn_la_options const& plcdn_la_opt,
		std::unordered_map<std::string, site_info> const& sitelist,
		std::unordered_map<std::string, nginx_domain_stat> & logstats);

/* override version for file
 * @param failed_line, failded_line for file @param file
 * @return 0 on success */
int do_nginx_log_stats(FILE * file, plcdn_la_options const& plcdn_la_opt,
		std::unordered_map<std::string, site_info> const& sitelist,
		std::unordered_map<std::string, nginx_domain_stat> & logstats, size_t & failed_line);
#endif /*_NGINX_LOG_ANALYSIS_H_*/

