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

//std::set<int> srs_log_stat::sids() const
//{
//	std::set<int> ret;
//	for(auto & item : urls){ ret.insert(item.first); };
//	for(auto & item : ips){ ret.insert(item.first); };
//	for(auto & item : obytes){ ret.insert(item.first); };
//	for(auto & item : ibytes){ ret.insert(item.first); };
//	return ret;
//}
//////////////////////////////////////////////////////////////////////////////////
int parse_srs_log_header_sid(char const * buff, char const * end)
{
	if(!buff || !end || end - buff < 1 || *buff != '[')
		return -1;
	auto p = strchr(buff + 1, '[');
	for(int i = 2; p && i > 0; --i) {
		p = strchr(++p, '[');
	}
	if(!p || p >= end)
		return -1;

	auto e = strchr(p + 1, ']');
	if(!e || e >= end || p + 1 >= e)
		return -1;

	return stoi(std::string(p + 1, e));
}

int parse_srs_log_header_time(char const * buff, char const * end, time_t & t)
{
	//'[2016-11-15 18:05:02.665]'
	if(!buff || !end || end - buff - 1 < 19 || *buff != '[')	/*'2016-11-15 18:05:02'*/
		return -1;
//	fprintf(stdout, "%s: __len=%ld__", __FUNCTION__, end - buff);
//	for(auto p = buff; p != end; ++p){
//		fprintf(stdout, "%c", *p);
//	}
//	fprintf(stdout, "____\n");
	tm my_tm;
	auto && timestr = std::string(buff + 1, buff + 1 + 19 + 1);
	auto result = strptime(timestr.c_str(), "%Y-%m-%d %H:%M:%S", &my_tm);
	if(!result)
		return -1;
	my_tm.tm_isdst = 0;
	t = mktime(&my_tm);

//	char buft1[32];
//	fprintf(stdout, "%s: _____%s_____\n", __FUNCTION__, time_group(t).c_str_r(buft1, sizeof(buft1)));
	return 0;
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
	stat.urls[logitem.trans.sid] = it_url->url;

	if(dstat._site_id == 0)
		find_site_id(g_sitelist, domain.c_str(), dstat._site_id, &dstat._user_id);

	auto find_ip_by_sid = [&logitem](srs_connect_ip const& item){ return item.sid == logitem.trans.sid;  };
	auto it_ip = std::find_if(ip_items.cbegin(), ip_items.cend(), find_ip_by_sid);
	if(it_ip == ip_items.end()) return -1;

	stat.ips[logitem.trans.sid] = it_ip->ip;

	stat.ibytes[logitem.trans.sid] += logitem.trans.ibytes;
	stat.obytes[logitem.trans.sid] += logitem.trans.obytes;
	return 0;
}

int do_srs_log_sid_stats(int sid, srs_sid_log & slog, srs_domain_stat & dstat,
		size_t & failed_line, size_t & trans_line)
{
//	fprintf(stdout, "%s: _____sid = %d, size = %zu, site_id = %d______\n", __FUNCTION__,
//			sid, slog._logs.size(), slog._site_id);
	dstat._site_id = slog._site_id;
	dstat._user_id = slog._user_id;

	std::vector<srs_trans> vec;
	auto & dslog = dstat._sid_log[sid];
	for(auto & log : slog._logs){
		if(!log.buff.empty())
			dslog += log.buff;

		srs_trans trans;
		memset(&trans, 0, sizeof(srs_trans));
		auto r = parse_srs_log_item_trans(sid, log, trans);
		if(r < 0){
			++failed_line;
			continue;	/*parse faield*/
		}
		auto & stat = dstat._stats[trans.time_stamp];
		/**
		 * FIXME: connection logs are also be put into stat._logs[time]
		 * if 'plcdn_la_options.format_split_srs_log' contains 'interval',
		 * then connection logs are lost for most of the split log files, @date 2016/12/05
		 */
		stat.logs.push_back(log);

		if(r == 0){
			stat.urls[sid] = slog._url;
			stat.ips[sid] = slog._ip;
			vec.push_back(trans);

			if(trans.ver == 1){
				stat.ibytes[sid] += trans.ibytes;
				stat.obytes[sid] += trans.obytes;
			}
			++trans_line;
		}
	}
	/*!
	 * FIXME: when rotate srs log, total line of trans log may < 2, in this condition
	 * ibytes and obytes can NOT be calculated, @author hongjun.liao <docici@126.com> @date 2016/12/28
	 */
	if(vec.size() < 2){
		fprintf(stderr, "%s: total srs_trans = %zu, < 2, rotate srs log too fast?\n", __FUNCTION__, vec.size());
		return 0;
	}
	/*sort first*/
	auto sort_by_timestamp = [](srs_trans const& a, srs_trans const& b){ return a.time_stamp < b.time_stamp; };
	std::sort(vec.begin(), vec.end(), sort_by_timestamp);

	/*calculate bytes for official srs format*/
	for(auto a = vec.begin(), b = ++vec.begin(); b != vec.end(); ++a, ++b){
		/*FIXME: there IS 'time' in official trans_log, parse and use that one?*/
		auto difft = difftime(b->time_stamp, a->time_stamp);
		auto ibytes = difft * ( difft < 30.001? b->ikbps_30s * difft / 8.0 : b->ikbps_5min * difft / 8.0 );
		auto obytes = difft * ( difft < 30.001? b->okbps_30s * difft / 8.0 : b->okbps_5min * difft / 8.0 );

		if(b->ver == 0){
			auto & stat = dstat._stats[b->time_stamp];
			stat.ibytes[sid] += ibytes;
			stat.obytes[sid] += obytes;
		}
	}
	return 0;
}

int parse_srs_log_item_conn(char const * buff, srs_connect_ip& ip, srs_connect_url & url, int& t)
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
		url.end = cm4[1].second;
//		buff[cm4[1].second - buff]  = '\0';
//		fprintf(stdout, "%s: ______url.url=%s___________\n", __FUNCTION__, url.url);
	}
	else
		t = 0;
	return 0;
}

int parse_srs_log_item_trans(int sid, srs_raw_log_t & rlog, srs_trans & trans)
{
	time_t time_stamp;
	auto status = parse_srs_log_header_time(rlog.first, rlog.second, time_stamp);
	if(status != 0) {
		//fprintf(stderr, "%s: _____parse time_stamp failed for buff = %s______\n", __FUNCTION__, buff);
		return -1;
	}
	trans.time_stamp = time_stamp;

	/* TODO: parse official format, @date 2016/12/13
	 * @NOTES: obytes and ibytes are NOT exist in official format
	 */
	/* FIXME: there are 3 okbps/ikbps, use which one? @see srs log official format,
	 * @date 2016/12/20 @author hongjun.liao <docici@126.com>
	 * */
	/*'(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?'*/
	static auto s2 = "(obytes=([0-9]+), ibytes=([0-9]+), )?okbps=([0-9]+),([0-9]+),([0-9]+), ikbps=([0-9]+),([0-9]+),([0-9]+)";
	static boost::regex r2{s2};
	boost::cmatch cm2;
	if(boost::regex_search(rlog.first, cm2, r2)) {
		rlog.type = 2;	/*FIXME: u may found a better way*/
		trans.sid = sid;

		char * end;
		if(cm2.size() == 9) {	/*custom format*/
			trans.ver = 1;
			trans.obytes = strtoul(cm2[1].str().c_str(), &end, 10);
			trans.ibytes = strtoul(cm2[2].str().c_str(), &end, 10);

			trans.okbps = strtoul(cm2[3].str().c_str(), &end, 10);
			trans.okbps_30s = strtoul(cm2[4].str().c_str(), &end, 10);
			trans.okbps_5min = strtoul(cm2[5].str().c_str(), &end, 10);

			trans.ikbps = strtoul(cm2[6].str().c_str(), &end, 10);
			trans.ikbps_30s = strtoul(cm2[7].str().c_str(), &end, 10);
			trans.ikbps_5min = strtoul(cm2[8].str().c_str(), &end, 10);
		}
		else if(cm2.size() == 7){	/*official format*/
			trans.ver = 0;
			trans.okbps = strtoul(cm2[1].str().c_str(), &end, 10);
			trans.okbps_30s = strtoul(cm2[2].str().c_str(), &end, 10);
			trans.okbps_5min = strtoul(cm2[3].str().c_str(), &end, 10);

			trans.ikbps = strtoul(cm2[4].str().c_str(), &end, 10);
			trans.ikbps_30s = strtoul(cm2[5].str().c_str(), &end, 10);
			trans.ikbps_5min = strtoul(cm2[6].str().c_str(), &end, 10);
		}
		return 0;
	}
	return 1;
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

