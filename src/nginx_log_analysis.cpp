/*!
 * This file is PART of nginx_log_analysis
 */
#include "nginx_log_analysis.h"
#include <stdio.h>
#include <time.h> /*strptime*/
#include <string.h> /*strncpy*/
#include "ipmap.h"		/*ipmap_nlookup*/
#include "net_util.h"	/*netutil_get_ip_str*/

int time_group::_sec = 300;
time_group::time_group(char const * strtime)
: _t(0)
{
	group(strtime);
}

time_group::time_group(time_t const& t)
: _t(0)
{
	group(t);
}

const char* time_group::c_str(char const * fmt) const
{
//	return ctime(&_t);
	static char buff[20] = "";
	if(!fmt) return NULL;
	strftime(buff, 20, fmt, localtime(&_t));
	return buff;
}

char * time_group::c_str_r(char * buff, size_t len, char const * fmt /*= "%Y-%m-%d %H:%M:%S"*/) const
{
	if(!fmt || !buff) return NULL;
	strftime(buff, len, fmt, localtime(&_t));
	return buff;
}

void time_group::group(const char* strtime)
{
	if(!strtime) return;
	tm my_tm;
	char const * result = strptime(strtime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	if(!result) return;

	my_tm.tm_isdst = 0;
	auto t = mktime(&my_tm);
//	fprintf(stdout, "%s: %s, t=%ld,_t=%ld\n", __FUNCTION__, ctime(&t), t, _t);
	return group(t);
}

void time_group::group(time_t const& t)
{
	size_t n = difftime(t, _t) / _sec;
	_t += n * _sec;
}
bool operator <(const time_group& one, const time_group& another)
{
	return one._t < another._t;
}

bool operator ==(const time_group& one, const time_group& another)
{
	return one._t == another._t;
}

time_group time_group::next() const
{
	auto ret(*this);
	ret._t += _sec;
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////
extern struct ipmap_ctx g_ipmap_ctx;

locisp_group::locisp_group(uint32_t ip/* = 0*/)
{
	strcpy(_locisp, "- -");
    auto isp = ipmap_nlookup(&g_ipmap_ctx, ip);
    if(isp){
		char ispbuff[32] = "";
		char const * str_isp, * fmt, *param;
    	str_isp = ipmap_tostr2(isp, ispbuff);
		bool f = strcmp(str_isp, "CN") && strcmp(str_isp, "CA") && strcmp(str_isp, "US");
		fmt = f? "%s -" : "%s";
		param = f? str_isp : ispbuff;
		snprintf(_locisp, 32, fmt, param);
    }
}

void locisp_group::loc_isp_c_str(char * buff, int len) const
{
	strncpy(buff, _locisp, len);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator ==(const locisp_group& one, const locisp_group& another)
{
	return strcmp(one._locisp, another._locisp) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
std::size_t std::hash<locisp_group>::operator()(locisp_group const& val) const
{
	std::size_t const h1 ( std::hash<std::string>{}(val._locisp) );	// or use boost::hash_combine
	return h1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
cutip_group::cutip_group(uint32_t ip)
{
	netutil_get_ip_str(ip, _cutip, sizeof(_cutip));
//	printf("%s: %s\n", __FUNCTION__, _cutip);
	char * c = strrchr(_cutip, '.');
	if(c) *c = '\0';
}

char const * cutip_group::c_str() const
{
	return _cutip;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool operator ==(const cutip_group& one, const cutip_group& another)
{
	return strcmp(one._cutip, another._cutip) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
std::size_t std::hash<cutip_group>::operator()(cutip_group const& val) const
{
	std::size_t const h1 ( std::hash<std::string>{}(val._cutip) );	// or use boost::hash_combine
	return h1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
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

 size_t log_stat::bytes(int code1, int code2/* = -1*/) const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes(code1, code2);
	}
	return ret;
}

 size_t log_stat::bytes_total() const
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

