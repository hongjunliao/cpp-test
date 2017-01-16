/*!
 * This file is PART of srs_log_analysis
 */

#include "srs_log_analysis.h"
#include "test_options.h"	/*plcdn_la_options*/
#include <string>				/*std::string*/
#include <unordered_map>		/*std::unordered_map*/
#include <boost/regex.hpp> 		/*regex_search*/
#include "net_util.h"			/*netutil_get_ip_from_str*/

#include "nginx_log_analysis.h"	/*site_info*/

/*plcdn_log_analysis/main.cpp*/
extern std::unordered_map<std::string, site_info> g_sitelist;
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;
//////////////////////////////////////////////////////////////////////////////////
size_t srs_log_stat::obytes_total() const
{
	size_t ret = 0;
	for(auto & it : obytes)
		ret += it.second;
	return ret;
}

size_t srs_log_stat::ibytes_total() const
{
	size_t ret = 0;
	for(auto & it : ibytes)
		ret += it.second;
	return ret;
}
//
//double srs_log_stat::avg_okbps() const
//{
////	size_t total = 0;
////	for(auto & it : okbps)
////		total += it.second;
////	return total * 1.0 / okbps.size();
//}
//
//double srs_log_stat::avg_ikbps() const
//{
////	size_t total = 0;
////	for(auto & it : ikbps)
////		total += it.second;
////	return total * 1.0 / ikbps.size();
//}
//
//double srs_log_stat::max_okbps() const
//{
////	double ret = 0;
////	for(auto & it : okbps){
////		if(it.second > ret)
////			ret = it.second;
////	}
////	return ret;
//}
//
//double srs_log_stat::max_ikbps() const
//{
////	double ret = 0;
////	for(auto & it : ikbps){
////		if(it.second > ret)
////			ret = it.second;
////	}
////	return ret;
//}
//
//double srs_log_stat::min_okbps() const
//{
////	double ret = 0;
////	for(auto & it : okbps){
////		if(it.second < ret)
////			ret = it.second;
////	}
////	return ret;
//}
//
//double srs_log_stat::min_ikbps() const
//{
////	double ret = 0;
////	for(auto & it : ikbps){
////		if(it.second < ret)
////			ret = it.second;
////	}
////	return ret;
//}

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
	for(auto psid = p + 1; psid != e; ++psid){
		if(!isdigit(*psid))
			return -1;
	}
	return std::stoi(std::string(p + 1, e));
}

int parse_srs_log_header_time(char const * buff, char const * end, time_t & t)
{
	//'[2016-11-15 18:05:02.665]'
	if(!buff || !end || end - buff - 1 < 19 || *buff != '['){	/*'2016-11-15 18:05:02'*/
		if(plcdn_la_opt.verbose > 3){
			fprintf(stdout, "%s: failed: [%s]\n", __FUNCTION__, buff);
		}
		return -1;
	}
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

	static auto const s1 = "RTMP client ip=([0-9.]+)";
	static auto const s2 = "(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?obytes=([0-9]+), ibytes=([0-9]+),"
				 " okbps=([0-9]+),[0-9]+,[0-9]+, ikbps=([0-9]+),[0-9]+,[0-9]+";
	static auto const s3 = "client disconnect peer\\. ret=[0-9]+";
	static auto const s4 = "\\][ \t]+connect app,[ \t]+tcUrl=", s4_vhost = "vhost=([^,]+),";
	static const boost::regex r1{s1}, r2{s2}, r3{s3}, r4{s4}, r4_vhost{s4_vhost};

	boost::cmatch cm1, cm2, cm3, cm4;
	if(boost::regex_search(buff, cm1, r1)) {
		log_type = 1;
		logitem.conn_ip.time_stamp = time_stamp;
		logitem.conn_ip.sid = sid;
		logitem.conn_ip.ip = netutil_get_ip_from_str(cm1[1].str().c_str());
	}
	else if(boost::regex_search(buff, r4) && boost::regex_search(buff, cm4, r4_vhost)) {
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
		size_t & failed_line, size_t & trans_line, bool& skip)
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
		if(log.second - log.first <= 0)
			continue;	/* why this happened? */
		srs_trans trans;
		memset(&trans, 0, sizeof(srs_trans));
		auto r = parse_srs_log_item_trans(sid, log, trans);
		if(r < 0){
			++failed_line;
			continue;	/*parse faield*/
		}
		if(r == 2)
			continue;
		auto & stat = dstat._stats[trans.time_stamp];
		stat.urls[sid] = slog._url;
		stat.ips[sid] = slog._ip;
		/*just for add a new item if needed*/
		stat.obytes[sid] += 0;
		stat.ibytes[sid] += 0;


		/**
		 * FIXME: connection logs are also be put into stat._logs[time]
		 * if 'plcdn_la_options.format_split_srs_log' contains 'interval',
		 * then connection logs are lost for most of the split log files, @date 2016/12/05
		 */
		stat.logs.push_back(log);

		if(r == 0){
			++trans_line;
			vec.push_back(trans);
			if(trans.ver == 1){
				stat.obytes[sid] += trans.obytes;
				stat.ibytes[sid] += trans.ibytes;
			}
		}
	}
	/*!
	 * FIXME: when rotate srs log, total line of trans log may < 2, in this condition
	 * ibytes and obytes can NOT be calculated, @author hongjun.liao <docici@126.com> @date 2016/12/28
	 */
	if(vec.size() < 2){
		skip = true;
		return 0;
	}
	skip = false;
	/*sort first*/
	auto sort_by_timestamp = [](srs_trans const& a, srs_trans const& b){ return a.time_stamp < b.time_stamp; };
	std::sort(vec.begin(), vec.end(), sort_by_timestamp);

//	for(auto const& item : vec){
//		fprintf(stdout, "%s: _____time=%lld, ikbps_30s=%zu, okbps_30s=%zu________________\n", __FUNCTION__,
//				item.time_stamp, item.ikbps_30s, item.okbps_30s);
//	}
	/*calculate bytes for official srs format*/
	for(auto a = vec.begin(), b = ++vec.begin(); b != vec.end(); ++a, ++b){
		if(b->ver == 0){
			/*FIXME: there IS 'time' in official trans_log, parse and use that one?*/
			auto difft = difftime(b->time_stamp, a->time_stamp);
			difft = (b->msec - a->msec) / 1000.0;	/* use time in official trans_log */
			auto okbps = (difft < 30.001? b->okbps_30s : b->okbps_5min);
			auto ikpbs = (difft < 30.001? b->ikbps_30s : b->ikbps_5min);
			auto obytes = 1024.0 * difft * okbps / 8.0;
			auto ibytes = 1024.0 * difft * ikpbs / 8.0;

			auto & stat = dstat._stats[b->time_stamp];
			stat.obytes[sid] += obytes;
			stat.ibytes[sid] += ibytes;
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
	static auto s_url = "connect app, tcUrl=([^,]+),", s_vhost = ",[ \t]*vhost=([^,]+),";
	static boost::regex r1{s1}, re_url{s_url}, re_vhost{s_vhost};

	boost::cmatch cm1, cm4_url, cm4_vhost;
	if(boost::regex_search(buff, cm1, r1)) {
		t = 1;
//		ip.time_stamp = time_stamp;
//		ip.sid = sid;
		ip.ip = netutil_get_ip_from_str(cm1[1].str().c_str());
	}
	else if(boost::regex_search(buff, cm4_url, re_url) &&
			boost::regex_search(buff, cm4_vhost, re_vhost)) {
		t = 2;
//		url.time_stamp = time_stamp;
//		url.sid = sid;
		url.url = cm4_url[1].first;
		url.end = cm4_url[1].second;

		url.domain = cm4_vhost[1].first;
		url.d_end = cm4_vhost[1].second;
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
	if(!is_time_in_range(time_stamp, plcdn_la_opt.begin_time, plcdn_la_opt.end_time))
		return 2;
	trans.time_stamp = time_stamp;

	/* TODO: parse official format, @date 2016/12/13
	 * @NOTES: obytes and ibytes are NOT exist in official format
	 */
	/* FIXME: there are 3 okbps/ikbps, use which one? @see srs log official format,
	 * @date 2016/12/20 @author hongjun.liao <docici@126.com>
	 * sample match result(total 10):
	 * official: '[okbps=0,0,0, ikbps=948,937,948][][][][0][0][0][948][937][948]'
	 * custom:   '[obytes=0, ibytes=19916, okbps=0,0,0, ikbps=478,556,472][obytes=0, ibytes=19916, ][0][19916][0][0][0][478][556][472]'
	 * */
	/*'(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?'*/
	static auto s2 = "time=([0-9]+), (msgs=[0-9]+, )?(obytes=([0-9]+), ibytes=([0-9]+), )?okbps=([0-9]+),([0-9]+),([0-9]+), "
			"ikbps=([0-9]+),([0-9]+),([0-9]+)";
	static boost::regex r2{s2};
	boost::cmatch cm2;
	if(boost::regex_search(rlog.first, cm2, r2)) {
		trans.ver = (cm2[3].str().empty()? 0 : 1);
//		fprintf(stdout, "%s:____________%zu__________________________________________________________________\n", __FUNCTION__, cm2.size());
//		for(auto & it : cm2){
//			fprintf(stdout, "[%s]", it.str().c_str());
//		}
//		fprintf(stdout, "\n%s:______________________________________________________________________________\n", __FUNCTION__);

		char * end;
		rlog.type = 2;	/*FIXME: u may found a better way*/
		trans.sid = sid;
		trans.msec = strtoul(cm2[1].str().c_str(), &end, 10);
		trans.obytes = trans.ibytes = 0;
		if(trans.ver == 1) {	/*custom format*/
			trans.obytes = strtoul(cm2[4].str().c_str(), &end, 10);
			trans.ibytes = strtoul(cm2[5].str().c_str(), &end, 10);
		}
		trans.okbps = strtoul(cm2[6].str().c_str(), &end, 10);
		trans.okbps_30s = strtoul(cm2[7].str().c_str(), &end, 10);
		trans.okbps_5min = strtoul(cm2[8].str().c_str(), &end, 10);

		trans.ikbps = strtoul(cm2[9].str().c_str(), &end, 10);
		trans.ikbps_30s = strtoul(cm2[10].str().c_str(), &end, 10);
		trans.ikbps_5min = strtoul(cm2[11].str().c_str(), &end, 10);
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

