/*!
 * This file is PART of srs_log_analysis
 */

#include "srs_log_analysis.h"
#include <string>				/*std::string*/
#include <unordered_map>		/*std::unordered_map*/
#include <boost/regex.hpp> 		/*regex_search*/
#include "net_util.h"			/*netutil_get_ip_from_str*/

#include "nginx_log_analysis.h"	/*site_info*/

/*plcdn_log_analysis/main.cpp*/
extern std::unordered_map<std::string, site_info> g_sitelist;

//////////////////////////////////////////////////////////////////////////////////
srs_sid_log::srs_sid_log(int sid)
: _site_id(0)
, _ip(0)
, _bytes(0)
{
	//none
}

srs_sid_log::operator bool() const
{
	return _site_id != 0 && _ip != 0;
}
//////////////////////////////////////////////////////////////////////////////////
int parse_srs_log_header_sid(char const * buff)
{
	auto p = strchr(buff, ']');
	for(int i = 3; p && i > 0; --i) { p = strchr(++p, '['); }
	auto end = p? strchr(p, ']') : NULL;
	if(!p || !end) return -1;
	return stoi(std::string(p + 1, end));
}

int parse_srs_log_header_time(char const * buff, time_t & t)
{
//	auto p = strchr(buff, ']');
//	auto end = p? strchr(p, ']') : NULL;
//	if(!p || !end) return -1;
//	return stoi(std::string(p + 1, end));
	return 1;
}

int parse_srs_log_header(char *& buff, time_t & time_stamp, int & sid)
{
	int index = 0;
	bool is_arg_start = false;
	for(auto p = buff, q = buff; ; ++q){
		if(*q == '['){
			is_arg_start = true;
			continue;
		}
		if(*q == ']'){
			if(!is_arg_start) return -1;
			*q = '\0';
			if(index == 0){
				tm my_tm;
				char const * result = strptime(p + 1, "%Y-%m-%d %H:%M:%S", &my_tm);
				if(!result) return -1;
				my_tm.tm_isdst = 0;
				time_stamp = mktime(&my_tm);
			}
			else if(index == 3){
				/*FIXME: sid CAN BE 0?*/
				sid = atoi(p + 1);
				buff = q + 1;
				break;
			}
			p = q + 1;
			++index;
			is_arg_start = false;
		}
	}
	return 0;
}

int parse_srs_log_item(char * buff, srs_log_item& logitem, int& log_type)
{
	time_t time_stamp;
	int sid;
	auto status = parse_srs_log_header(buff, time_stamp, sid);
	if(status != 0) return -1;

	static auto s1 = "RTMP client ip=([0-9.]+)";
	static auto s2 = "(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?obytes=([0-9]+), ibytes=([0-9]+),"
				 " okbps=([0-9]+),[0-9]+,[0-9]+, ikbps=([0-9]+),[0-9]+,[0-9]+";
	static auto s3 = "client disconnect peer\\. ret=[0-9]+";
	static auto s4 = "connect app, tcUrl=(.+), pageUrl=";
	static boost::regex r1{s1}, r2{s2}, r3{s3}, r4{s4};

	boost::cmatch cm1, cm2, cm3, cm4;
	if(boost::regex_search(buff, cm1, r1)) {
		log_type = 1;
		logitem.conn_ip.time_stamp = time_stamp;
		logitem.conn_ip.sid = sid;
		logitem.conn_ip.ip = netutil_get_ip_from_str(cm1[1].str().c_str());
	}
	else if(boost::regex_search(buff, cm4, r4)) {
		log_type = 4;
		logitem.conn_url.time_stamp = time_stamp;
		logitem.conn_url.sid = sid;

		logitem.conn_url.url = cm4[1].first;
		buff[cm4[1].second - buff]  = '\0';
//		fprintf(stdout, "%s: ______conn_url.url=%s___________\n", __FUNCTION__, logitem.conn_url.url);
	}
	else if(boost::regex_search(buff, cm2, r2)) {
		log_type = 2;
		memset(&logitem.trans, 0, sizeof(logitem.trans));
		logitem.trans.time_stamp = time_stamp;
		logitem.trans.sid = sid;
		char * end;
		logitem.trans.obytes = strtoul(cm2[1].str().c_str(), &end, 10);
		logitem.trans.ibytes = strtoul(cm2[2].str().c_str(), &end, 10);
		logitem.trans.okbps = strtoul(cm2[3].str().c_str(), &end, 10);
		logitem.trans.ikbps = strtoul(cm2[4].str().c_str(), &end, 10);
	}
	else if(boost::regex_search(buff, cm3, r3)) {
		log_type = 3;
	}
	else{
		log_type = 0;
	}
	return 0;
}

int do_srs_log_stats(srs_log_item const& logitem, int log_type,
		std::vector<srs_connect_ip> const& ip_items,
		std::vector<srs_connect_url> const& url_items,
		std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	if(log_type != 2) return -1;

	auto find_url_by_sid = [&logitem](srs_connect_url const& item){ return item.sid == logitem.trans.sid;  };
	auto it_url = std::find_if(url_items.cbegin(), url_items.cend(), find_url_by_sid);
	if(it_url == url_items.end())
		return -1;

	/*sample: 'rtmp://127.0.0.1:1359/'*/
	boost::cmatch cm;
	auto f = boost::regex_search(it_url->url, cm, boost::regex("://([^/:]+)(?::[0-9]+)?/"));
	if(!f) return -1;

	auto && domain = cm[1].str();
	auto & dstat = logstats[domain];
	auto & stat = dstat._stats[logitem.trans.time_stamp];
	stat.sid = logitem.conn_url.sid;
	stat.url = it_url->url;

	if(dstat._site_id == 0)
		find_site_id(g_sitelist, domain.c_str(), dstat._site_id, &dstat._user_id);

	auto find_ip_by_sid = [&logitem](srs_connect_ip const& item){ return item.sid == logitem.trans.sid;  };
	auto it_ip = std::find_if(ip_items.cbegin(), ip_items.cend(), find_ip_by_sid);
	if(it_ip == ip_items.end()) return -1;

	stat.ip = it_ip->ip;

	stat.ibytes += logitem.trans.ibytes;
	stat.obytes += logitem.trans.obytes;
	return 0;
}

int do_srs_log_sid_stats(int sid, srs_sid_log const & slog, srs_domain_stat & dstat)
{
	for(auto & log : slog._logs){
		srs_trans trans;
		auto buff = log.first;
		auto r = parse_srs_log_item_trans(buff, trans);
		if(r != 0)
			continue;
		/*TODO: change srs_sid_log.type*/
//		log.type = 2;

		dstat._site_id = slog._site_id;
		dstat._user_id = slog._user_id;
		auto & stat = dstat._stats[trans.time_stamp];

		stat.sid = sid;
		stat.url = slog._url;
		stat.ip = slog._ip;

		stat.ibytes += trans.ibytes;
		stat.obytes += trans.obytes;

		stat._logs.push_back(log);
	}
	return 0;
}

int parse_srs_log_item_conn(char * buff, srs_connect_ip& ip, srs_connect_url & url, int& t)
{
//	printf("%s: ___%s____\n", __FUNCTION__, buff);
//	time_t time_stamp;
//	int sid;
//	auto status = parse_srs_log_header(buff, time_stamp, sid);
//	if(status != 0) return -1;

	static auto s1 = "RTMP client ip=([0-9.]+)";
	static auto s4 = "connect app, tcUrl=(.+), pageUrl=";
	static boost::regex r1{s1}, r4{s4};

	boost::cmatch cm1, cm4;
	if(boost::regex_search(buff, cm1, r1)) {
		t = 1;
//		ip.time_stamp = time_stamp;
//		ip.sid = sid;
		ip.ip = netutil_get_ip_from_str(cm1[1].str().c_str());
	}
	else if(boost::regex_search(buff, cm4, r4)) {
		t = 2;
//		url.time_stamp = time_stamp;
//		url.sid = sid;
		url.url = cm4[1].first;
		buff[cm4[1].second - buff]  = '\0';
//		fprintf(stdout, "%s: ______conn_url.url=%s___________\n", __FUNCTION__, logitem.conn_url.url);
	}
	else
		t = 0;
	return 0;
}

int parse_srs_log_item_trans(char * buff, srs_trans & trans)
{
	time_t time_stamp;
	auto status = parse_srs_log_header_time(buff, time_stamp);
	if(status != 0) return -1;

	static auto s2 = "(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?obytes=([0-9]+), ibytes=([0-9]+),"
				 " okbps=([0-9]+),[0-9]+,[0-9]+, ikbps=([0-9]+),[0-9]+,[0-9]+";
	static boost::regex r2{s2};
	boost::cmatch cm2;
	if(boost::regex_search(buff, cm2, r2)) {
		trans.time_stamp = time_stamp;
		char * end;
		trans.obytes = strtoul(cm2[1].str().c_str(), &end, 10);
		trans.ibytes = strtoul(cm2[2].str().c_str(), &end, 10);
		trans.okbps = strtoul(cm2[3].str().c_str(), &end, 10);
		trans.ikbps = strtoul(cm2[4].str().c_str(), &end, 10);
	}
	return 0;
}
int parse_domain_from_url(const char* url, char* domain)
{
	/*sample: 'rtmp://127.0.0.1:1359/'*/
	boost::cmatch cm;
	auto f = boost::regex_search(url, cm, boost::regex("://([^/:]+)(?::[0-9]+)?/"));
	if(!f) return -1;

	auto length = cm.length(1);
	strncpy(domain, cm[1].first, length);
	domain[length] = '\0';
	return 0;
}
