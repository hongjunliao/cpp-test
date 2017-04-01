/*!
 * This file is PART of nginx_log_analysis
 */
#include <fnmatch.h>	/*fnmatch*/
#include <plcdn_la_ngx.h>
#include <stdio.h>
#include <time.h> /*strptime*/
#include <string.h> /*strncpy*/
#include <time.h> /*strptime*/
#include <algorithm> /* std::find */
#include <numeric>	/* std::accumulate */
#include "string_util.h"	/* sha1sum_r */
#include "net_util.h"	/*netutil_get_ip_str*/

/* parse nginx_log $request_uri field, return url, @param cache_status: MISS/MISS0/HIT,...
 * @param mode:
 * mode 0, url endwith ' ', reserve all, e.g. "POST /zzz.asp;.jpg HTTP/1.1", return "/zzz.asp;.jpg"
 * mode 1, url endwith '?'(if no '?', then endwith ' '), ignore parameters, e.g. "GET /V3/?page_id=1004&cid=1443 HTTP/1.1", return "/V3/"
 * mode 2, custom, @param cache_status required
 * */
static int parse_nginx_log_request_uri_method(char * buf,  char const * & request_method, int * mlen, char const *& request_uri, int * len,
		char const * cache_status, int mode = 2);
static char * parse_nginx_log_request_uri_url(char * request_uri, int * len, char const * cache_status, int mode = 2);

static int parse_nginx_log_request_uri_method(char * buf,  char  const  * & request_method, int * mlen, char const  *& request_uri, int * len,
		char const * cache_status, int mode /*= 2*/)
{
	request_uri = parse_nginx_log_request_uri_url(buf, len, cache_status, mode);
	if(!request_uri)
		return -1;
	request_method = buf;
	auto c = strchr(buf, ' ');
	if(!c)
		return -1;
	*c = '\0';
	if(mlen)
		*mlen = c - buf;
	return 0;
}

static char * parse_nginx_log_request_uri_url(char * request_uri, int * len, char const * cache_status, int mode/* = 2*/)
{
	auto url = strchr(request_uri, '/'); /*"GET /abc?param1=abc"*/
	if(!url) return NULL;
	auto pos = strchr(url, ' ');
	if(mode == 1){
		auto p = strchr(url, '?');
		if(p) pos = p;
	}
	else if(mode == 2){
		auto miss0 = strrchr(cache_status, '0');
		auto pos1 = strchr(url, '?'), pos2 = strchr(url, ';');
		if(miss0 && (pos1 || pos2)){
			if(pos1 && pos2)
				pos = std::min(pos1, pos2);
			else{
				pos = (pos1? pos1 : pos2);
			}
		}
	}
	auto length = pos - url;
	url[length] = '\0';
	if(len)
		*len = length;
	return url;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
locisp_stat& locisp_stat::operator+=(locisp_stat const& another)
{
	for(auto const& item : another._code){
		auto & s = _code[item.first];
		s.access += item.second.access;
		s.bytes += item.second.bytes;
		s.access_m += item.second.access_m;
		s.bytes_m += item.second.bytes_m;
	}
	_svg.insert(_svg.end(), another._svg.begin(), another._svg.end());
	return *this;
}
double locisp_stat_fpt(locisp_stat const& stat)
{
	return !stat._fpt.empty()? std::accumulate(stat._fpt.begin(), stat._fpt.end(), 0.0) / stat._fpt.size() : 0;
}

double locisp_stat_svg(locisp_stat const& stat)
{
	return !stat._svg.empty()? std::accumulate(stat._svg.begin(), stat._svg.end(), 0.0) / stat._svg.size() : 0;
}

void locisp_stat_access_bytes(locisp_stat const& stat,
		size_t & access, size_t & bytes, size_t & access_m, size_t & bytes_m)
{
	for(auto & item : stat._code){
		auto & i = item.second;
		access += i.access;
		bytes += i.bytes;
		access_m += i.access_m;
		bytes_m += i.bytes_m;
	}
}

void locisp_stat_access_bytes_m(locisp_stat const& stat, size_t & access_m, size_t & bytes_m)
{
	for(auto & item : stat._code){
		auto & i = item.second;
		access_m += i.access_m;
		bytes_m += i.bytes_m;
	}
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

nginx_raw_log::nginx_raw_log(std::pair<char *, char *> const& val)
/*: first(val.first), second(val.second)*/
{
	first = val.first;
	second = val.second;
}

nginx_raw_log::nginx_raw_log(std::string const& val)
: buff(val)
{
	first = second = NULL;
}

nginx_raw_log::nginx_raw_log(char const* val)
: buff(val)
{
	first = second = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
nginx_log_stat::nginx_log_stat()
/* FIXME: BUG founded!!!
 * bugname: BUG_srs_in_out_not_init_nginx_flow_table_20170224
 * @desc: if srs_in,srs_out NOT init, then value NOT correct in nginx_flow_table for last 2 lines(order by datetime)
 *        see bugs/BUG_srs_in_out_not_init_nginx_flow_table_20170224
 * @author: hongjun.liao <docici@126.com>
 * @date: 2017/02/24 */
: srs_in(0), srs_out(0)
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
	size_t access_m = 0, bytes_m = 0;
	for(auto & item : _locisp_stats){
		locisp_stat_access_bytes_m(item.second, access_m, bytes_m);
	}
	return access_m;
}

nginx_log_stat& nginx_log_stat::operator+=(nginx_log_stat const& another)
{
	for(auto const& item : another._url_stats){
		_url_stats[item.first] += item.second;
	}
	for(auto const& item : another._locisp_stats)
		_locisp_stats[item.first] += item.second;
	return *this;
}

bool nginx_log_stat::empty() const
{
	return _url_stats.empty() && _ip_stats.empty() && _cuitip_stats.empty() &&
			_locisp_stats.empty() && srs_in == 0 && srs_out == 0;
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
		fprintf(stderr, "%s: fopen siteuidlist file %s failed\n", __FUNCTION__, file);
		return -1;
	}
	site_info row;
	char domain[128];
	char buf[1024];
	while(fgets(buf, sizeof(buf), f)){
		int n = sscanf(buf, "%d%d%s%d",
				&row.site_id, &row.user_id, domain, &row.is_top);
		if(n != 4)
			continue;
		sitelist[domain] = row;
	}
	fclose(f);

//	for(auto const & item : sitelist){
//		auto & s = item.second;
//		fprintf(stdout, "%s:____%d____%d____%s____%d____\n", __FUNCTION__,
//				s.site_id, s.user_id, item.first.c_str(), s.is_top);
//	}
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

int do_parse_nginx_log_item(std::pair<char const *, char const *> * fields, char const * szitem, char delim /*= '\0'*/)
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
//				*q = '\0';
				fields[field_count++] = std::make_pair(p, q);
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
			fields[field_count++] = std::make_pair(p, q);;
			auto c = *q;
//			*q = '\0';
			if(c == delim){
				break;
			}
			p = q + 1;
		}
	}
//	for(int i  = 0; i < field_count; ++i){
//		fprintf(stdout, "%s: argv[%02d]:", __FUNCTION__, i);
//		for(auto p = fields[i].first; p != fields[i].second; ++p)
//			fprintf(stdout, "%c", *p);
//		fprintf(stdout, "\n");
//	}
	return 0;
error_return:
	return -1;
}

int do_parse_nginx_log_item(char** fields, char*& szitem, char const * v[2], char delim/* = '\0'*/)
{
	char const * ch = 0;
	int field_count = 0;
	auto q = szitem;
	for(auto p = q; ; ++q){
		if(*q == delim){
			*q = '\0';
			fields[field_count++] = p;
			break;
		}
		auto c = strchr(v[0], *q);
		if(c && !ch){ /* border_begin */
			if(!ch){
				ch = c;
				p = q + 1;
			}
			continue;
		}
		if(ch && *q == v[1][ch - v[0]]){ /* border_end */
			ch = 0;
			*q = '\0';
			fields[field_count++] = p;
			if(*(q + 1) == ' '){
				++q;
				p = q + 1;
			}
			continue;
		}
		if(*q == ' '){	/* separator */
			if(ch)
				continue;
			*q = '\0';
			fields[field_count++] = p;
			p = q + 1;
		}
	}
//	for(int i  = 0; i < field_count; ++i){
//		fprintf(stdout, "%s: argv[%02d]: %s\n", __FUNCTION__, i, fields[i]);
//	}
	return ch? -1 : 0;
}

/* split @param szitem, by ' ', insert '\0' and append to @param fields[n] */
static void split_string(char ** fields, int& n, char *& szitem, char delim = '\0')
{
	auto q = szitem;
	for(auto p = q; ; ++q){
		if(*q == delim){
			*q = '\0';
			fields[n++] = p;
			break;
		}
		if(*q != ' ')
			continue;
		*q = '\0';
		fields[n++] = p;
		p = q + 1;
	}
}

int do_parse_nginx_log_item(char** fields, char*& szitem, char const * v[2], std::vector<int> const& n, char delim/* = '\0'*/)
{
	char const * ch = 0;
	int field_count = 0;
	auto q = szitem;
	for(auto p = q; ; ++q){
		if(*q == delim){
			*q = '\0';
			if(std::find(n.begin(), n.end(), field_count) != std::end(n))
				split_string(fields, field_count, p, '\0');
			else
				fields[field_count++] = p;
			break;
		}
		auto c = strchr(v[0], *q);
		if(c && !ch){ /* border_begin */
			if(!ch){
				ch = c;
				p = q + 1;
			}
			continue;
		}
		if(ch && *q == v[1][ch - v[0]]){ /* border_end */
			ch = 0;
			*q = '\0';
			if(std::find(n.begin(), n.end(), field_count) != std::end(n))
				split_string(fields, field_count, p, '\0');
			else
				fields[field_count++] = p;
			if(*(q + 1) == ' '){
				++q;
				p = q + 1;
			}
			continue;
		}
		if(*q == ' '){	/* separator */
			if(ch)
				continue;
			*q = '\0';
			if(std::find(n.begin(), n.end(), field_count) != std::end(n))
				split_string(fields, field_count, p, '\0');
			else
				fields[field_count++] = p;
			p = q + 1;
		}
	}
	return ch? -1 : 0;
}

static int split_string(std::pair<char const *, char const *> * fields, size_t  fl, size_t & fc, char const * szitem, size_t bl)
{
	auto q = szitem;
	for(auto p = q; ; ++q){
		if(q == szitem + bl){
			if(fc == fl)
				return -1;
			fields[fc++] = std::make_pair(p, q);
			break;
		}
		if(*q != ' ')
			continue;
		if(fc == fl)
			return -1;
		fields[fc++] = std::make_pair(p, q);
		p = q + 1;
	}
	return 0;
}

static inline int int_cmp(void const * a, void const * b)
{
	return *(int const *)a - *(int const *)b;
}

int do_parse_nginx_log_item(std::pair<char const *, char const *> * fields, size_t & fl, int rn,
		char const * szitem, size_t bl,
		char const * v[2], int n, char delim/* = '\n'*/)
{
	char const * ch = 0;
	size_t fc = 0;
	auto q = szitem;
	for(auto p = q; ; ++q){
		if(q == szitem + bl){
			if(p != q)
				return -1;
			break;
		}
		auto c = strchr(v[0], *q);
		if(c && !ch){ /* border_begin */
			if(!ch){
				ch = c;
				p = q + 1;
			}
			continue;
		}
		if(ch && *q == v[1][ch - v[0]]){ /* border_end */
			ch = 0;
			if(fc == fl)
				return -2;
			if((int)fc % rn == n)
				split_string(fields, fl, fc, p, q - p);
			else
				fields[fc++] = std::make_pair(p, q);
			if(*(q + 1) == ' '){
				++q;
				p = q + 1;
			}
			continue;
		}
		if(*q == ' '){	/* separator */
			if(ch)
				continue;
			if(fc == fl)
				return -2;
			fields[fc++] = std::make_pair(p, q);;
			p = q + 1;
		}
		if(*q == delim){	/* end of line */
			if(ch)
				return -1;
			if(fc == fl)
				return -2;
			fields[fc++] = std::make_pair(p, q);
			auto lrn = (int)fc % rn;
			if(lrn > 0){
				for(; lrn != rn; ++lrn){
					fields[fc++] = std::make_pair<char const *, char const *>(0, 0);
					if(fc == fl)
						return -2;
				}
			}
			p = q + 1;
		}
	}
	fl = fc;
//	for(size_t i  = 0; i < fc; ++i){
//		fprintf(stdout, "%s: argv[%zu]:", __FUNCTION__, i);
//		if(!fields[i].first){
//			fprintf(stdout, "\\0\n");
//			continue;
//		}
//		for(auto p = fields[i].first; p != fields[i].second; ++p){
//			if(*p == '\n')
//				fprintf(stdout, "\\n");
//			else
//				fprintf(stdout, "%c", *p);
//		}
//		fprintf(stdout, "\n");
//	}

	return ch? -1 : 0;
}

/* @param hits sample value: 'STALE|UPDATING|REVALIDATED|HIT', @see plcdn_la_options.nginx_hit
 * NOTE: '|' separated
 * @return 0 on YES */
static int nginx_log_item_is_hit(char const * hits, char const * hit)
{
	auto end = hits + strlen(hits);
	for(auto p = hits, q = p; q != end; p = q + 1){
		q = strchr(p, '|');
		if(!q)
			q = end;
		if(strncmp(hit, p, q - p) == 0)
			return 0;
	}
	return -1;
}

int parse_log_item(log_item & item, char *& logitem, char delim, int parse_url_mode, char const * nginx_hit)
{
	memset(&item, 0, sizeof(log_item));
	item.beg = logitem;
	char *items[40] = { 0 };
	int result = do_parse_nginx_log_item(items, logitem, delim);
	if (result != 0) {
		return 1;
	}
	item.end = logitem;

	item.domain = items[0];
//	item.client_ip_2 = items[1];
	item.client_ip = netutil_get_ip_from_str(items[1]);
	if (item.client_ip == 0)
		return 1;

	char * end;
	item.request_time = strtoul(items[2], &end, 10);
	/*format: [17/Sep/2016:00:26:08 +0800]*/
	tm my_tm;
	if (!strptime(items[4] + 1, "%d/%b/%Y:%H:%M:%S", &my_tm))
		return -1;
	my_tm.tm_isdst = 0;
	item.time_local = mktime(&my_tm);

	auto ret = parse_nginx_log_request_uri_method(items[6], item.request_method,
			NULL, item.request_url, NULL, items[3], parse_url_mode);
	if (ret != 0)
		return -1;

	char const * p = items[8];
	item.bytes_sent = strtoul(p, &end, 10);
	item.status = atoi(items[7]);
	item.is_hit = (nginx_log_item_is_hit(nginx_hit, items[3]) == 0);
	if(items[16])
		item.response_time =  atoi(items[16]);
	return 0;
}

int do_nginx_log_stats(log_item const& item, plcdn_la_options const& plcdn_la_opt,
		std::unordered_map<std::string, site_info> const& sitelist,
		std::unordered_map<std::string, nginx_domain_stat> & logstats)
{
	auto & dstat = logstats[item.domain];
	if(dstat._site_id == 0)
		find_site_id(sitelist, item.domain, dstat._site_id, &dstat._user_id);

	auto len = strlen(item.request_url);
	char buff[64] = "";
	sha1sum_r(item.request_url, len, buff);
	dstat._url_key[buff] = item.request_url;

	auto & logsstat = dstat._stats[item.time_local];

	/*if NOT required, we needn't statistics it*/
	if(plcdn_la_opt.output_nginx_flow || plcdn_la_opt.output_file_url_popular || plcdn_la_opt.output_file_http_stats){
		url_stat& urlstat = logsstat._url_stats[buff];
		++urlstat._status[item.status];
		urlstat._bytes[item.status] += item.bytes_sent;
		urlstat.status = (item.status != 404? 0 : 1);
	}

	if(plcdn_la_opt.output_file_ip_popular || plcdn_la_opt.output_file_ip_slowfast){
		ip_stat& ipstat =logsstat._ip_stats[item.client_ip];
		ipstat.bytes += item.bytes_sent;
		ipstat.sec += item.request_time;
		++ipstat.access;

	}
	if(plcdn_la_opt.output_file_cutip_slowfast){
		auto & cutipstat = logsstat._cuitip_stats[item.client_ip];
		cutipstat.bytes += item.bytes_sent;
		cutipstat.sec += item.request_time;
	}
	locisp_stat& listat = logsstat._locisp_stats[item.client_ip];
	/* svg_speed */
	if(item.client_ip != netutil_get_ip_from_str("127.0.0.1") && item.request_time != 0
			&& ! (item.status < 200 || item.status > 299 || strncmp(item.request_method, "GET", 3)  != 0) ){
		auto s = item.bytes_sent * 1.0  / item.request_time;
		listat._svg.push_back(s);
		listat. _fpt.push_back(item.response_time);
	}
	/*FIXME, @date 2016/11/11*/
//		if(plcdn_la_opt.enable_devicelist_filter &&  g_devicelist[item.client_ip_2] != 0)
//			return 0;
	auto & cstats = listat._code[item.status];
	cstats.bytes += item.bytes_sent;
	++cstats.access;
	if(!item.is_hit){
		cstats.bytes_m += item.bytes_sent;
		++cstats.access_m;
	}

	/* @NOTES: push only when plcdn_la_opt.work_mode == 0 !!!  */
	if(plcdn_la_opt.work_mode == 0 && item.beg && item.end)
		logsstat._logs.push_back(std::make_pair<>(item.beg, item.end));
	return 0;
}

int do_nginx_log_stats(FILE * file, plcdn_la_options const& plcdn_la_opt,
		std::unordered_map<std::string, site_info> const& sitelist,
		std::unordered_map<std::string, nginx_domain_stat> & logstats, size_t & failed_line)
{
//	fprintf(stderr, "%s: file = %p\n", __FUNCTION__, file);
	if(!file) return -1;

	log_item item;
    char buf[1024 * 64];	/* max length of 1 row */
    size_t n = 0;
    while (fgets(buf, sizeof(buf), file)){
		++n;
		auto * p = buf;
		int result = parse_log_item(item, p, '\n', plcdn_la_opt.parse_url_mode, plcdn_la_opt.nginx_hit);
    	if(result != 0){
    		if(plcdn_la_opt.verbose > 4)
    			fprintf(stderr, "%s: do_parse_nginx_log_item failed, line=%zu, skip\n", __FUNCTION__, n);
    		++failed_line;
    		continue;
    	}
    	result = do_nginx_log_stats(item, plcdn_la_opt, sitelist, logstats);
    	if(result != 0)
    		++failed_line;
    }
    return 0;
}
