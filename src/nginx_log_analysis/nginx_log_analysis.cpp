/*!
 * This file is PART of nginx_log_analysis
 */
#include "nginx_log_analysis.h"
#include <fnmatch.h>	/*fnmatch*/
#include <stdio.h>
#include <time.h> /*strptime*/
#include <string.h> /*strncpy*/
#include <time.h> /*strptime*/
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

time_t time_group::t() const
{
	return _t;
}
char * time_group::c_str_r(char * buff, size_t len, char const * fmt /*= "%Y%m%d%H%M"*/) const
{
	if(!fmt || !buff) return NULL;
	tm t;
	strftime(buff, len, fmt, localtime_r(&_t, &t));
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
	size_t c1 = (_bytes.count(code1) != 0? _bytes.at(code1) : 0);
	size_t c2 = (_bytes.count(code2) != 0? _bytes.at(code2) : 0);
	return c1 + c2;
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
#ifdef ENABLE_IPMAP
struct ipmap_ctx locisp_group::_ipmap_ctx;
#endif //ENABLE_IPMAP

#ifdef ENABLE_IPMAP
int locisp_group::load_ipmap_file(char const * ipmap_file)
{
	return 0 == ipmap_load(ipmap_file, &_ipmap_ctx, 1)? 0 : 1;
}
#endif //ENABLE_IPMAP

locisp_group::locisp_group(uint32_t ip/* = 0*/)
{
	strcpy(_locisp, "- -");
#ifdef ENABLE_IPMAP
	/*FIXME: ipmap_nlookup, ipmap_alookup return NOT same*/
//    auto isp = ipmap_nlookup(&g_ipmap_ctx, ip);
	char buf[32];
	auto isp = ipmap_alookup(&_ipmap_ctx, netutil_get_ip_str(ip, buf, sizeof(buf)));
    if(isp){
		char ispbuff[32] = "";
    	auto str_isp = ipmap_tostr2(isp, ispbuff);
		bool f = strlen(str_isp) == 2 && strcmp(str_isp, "CN") && strcmp(str_isp, "CA") && strcmp(str_isp, "US");
		f? snprintf(_locisp, 32, "%s -", str_isp): snprintf(_locisp, 32, "%s", ispbuff);;

//		fprintf(stdout, "%s: str_isp=%s, ispbuff=%s\n", __FUNCTION__, str_isp, ispbuff);
    }
#endif //ENABLE_IPMAP
}

char * locisp_group::loc_isp_c_str(char * buff, int len) const
{
	strncpy(buff, _locisp, len);
	return buff;
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

cutip_group::cutip_group(char const * cutip/* = NULL*/)
{
	if(!cutip)
		return;
	strncpy(_cutip, cutip, sizeof(_cutip));
	auto len = strlen(cutip);
	if(len < 12){
		_cutip[len] = '\0';
	}
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
nginx_log_stat::nginx_log_stat()
: _bytes_m(0)
{
	//none
}

size_t nginx_log_stat::access_total() const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access_total();
	}
	return ret;
}

 size_t nginx_log_stat::bytes(int code1, int code2/* = -1*/) const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes(code1, code2);
	}
	return ret;
}

 size_t nginx_log_stat::bytes_total() const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.bytes_total();
	}
	return ret;
}

size_t nginx_log_stat::access(int code1, int code2/* = -1*/) const
{
	size_t ret = 0;
	for(auto it = _url_stats.begin(); it !=_url_stats.end(); ++it){
		ret += it->second.access(code1, code2);
	}
	return ret;
}

size_t nginx_log_stat::access_m() const
{
	size_t ret = 0;
	for(auto & item : _access_m){
		ret += item.second;
	}
	return ret;
}

nginx_log_stat& nginx_log_stat::operator+=(nginx_log_stat const& another)
{
	for(auto const& item : another._url_stats){
		_url_stats[item.first] += item.second;
	}
	for(auto & item : another._access_m)
		_access_m[item.first] += item.second;
	_bytes_m += another._bytes_m;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int find_site_id(std::unordered_map<std::string, site_info> const& sitelist,
		const char* site, int & siteid, int * user_id)
{
	if(!site || !site[0])
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
	if(!si){
		fprintf(stderr, "%s: site_id NOT found for '%s'\n", __FUNCTION__, site);
		return -1;
	}
	siteid = si? si->site_id : 0;
	if(user_id)
		*user_id = si? si->user_id : 0;
	return 0;
}


int load_sitelist(char const* file, std::unordered_map<std::string, site_info>& sitelist)
{
	if(!file || !file[0]) return -1;
	FILE * f = fopen(file, "r");
	if(!f){
		fprintf(stderr, "%s: fopen file %s failed\n", __FUNCTION__, file);
		return -1;
	}
	char data[1024] = "";
	while(fgets(data, sizeof(data), f)){
		if(data[0] == '\n') continue;
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

bool is_time_in_range(time_t const& t, time_t const& begin, time_t const& end)
{
	if(begin != 0 && end != 0)
		return t >= begin && t < end;
	if(begin == 0 && end == 0)
		return true;
	if(begin != 0)
		return t >= begin;
	if(end != 0)
		return t < end;
	return false;	/* never comes here */
}

int do_parse_nginx_log_item(char** fields, char*& szitem, char delim/* = '\0'*/)
{
//	for(char * ch = szitem; ; ++ch) { fprintf(stdout, "%c", *ch); if(*ch == delim) break; }
	auto arg_start = false;
	int field_count = 0;

	auto q = szitem;
	for(auto p = szitem; ; ++q){
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
			auto c = *q;
			*q = '\0';
			if(c == delim){
				break;
			}
			p = q + 1;
		}
	}
	szitem = q;
//	for(int i  = 0; i < field_count; ++i){
//		fprintf(stdout, "%s: argv[%02d]: %s\n", __FUNCTION__, i, fields[i]);
//	}

	return 0;
error_return:
	fields[0] = '\0';
	return -1;
}
