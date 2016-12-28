/*!
 * This file is PART of srs_log_analysis
 * data structs
 */
#ifndef _SRS_LOG_ANALYSIS_H_
#define _SRS_LOG_ANALYSIS_H_
#include "nginx_log_analysis.h"	/*time_group*/
#include <time.h>	/*time_t*/
#include <stdint.h>	/*uint32_t*/
#include <string>	/*std::string*/
#include <vector>	/*std::vector*/
#include <map>	/*std::map*/
#include <set>	/*std::set*/
/*declares*/
struct srs_connect_ip;
struct srs_connect_url;
struct srs_trans;
struct srs_disconnect;
union srs_log_item;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*srs log_type*/
/*conect_ip, sample: '[2016-11-03 11:27:32.736][trace][21373][105] RTMP client ip=127.0.0.1'*/
struct srs_connect_ip
{
	time_t time_stamp;
	int sid;
	uint32_t ip;
};

/*connect_url, sample:
 * '[2016-10-20 17:13:08.058][trace][20009][114] connect app, tcUrl=rtmp://192.168.212.164:1935/live, pageUrl=, \
 * swfUrl=, schema=rtmp, vhost=__defaultVhost__, port=1935, app=live, args=null'*/
struct srs_connect_url
{
	time_t time_stamp;
	int sid;
	/*FIXME: change to std::string?*/
	char const * url;
	char const * end;	/*',' yet*/
};

/*! srs log_type transform
 * in official srs log format, there's NO 'ibytes=' and 'obytes='(@see parse_srs_log_item_trans, do_srs_log_sid_stats),
 * if is official, ver = 0; if with ibytes and obytes, ver = 1
 * In a srs log file, there should only have one format for trans log
 */
struct srs_trans
{
	int ver;
	time_t time_stamp;
	int sid;
	size_t ibytes;
	size_t obytes;
	/*!
	 * in official srs log format, kbps have 3 values: realtime(?), in 30s, in 5min
	 */
	size_t ikbps, okbps;
	size_t ikbps_30s, okbps_30s;
	size_t ikbps_5min, okbps_5min;

};

/*srs log_type disconnect*/
struct srs_disconnect
{
	time_t time_stamp;
	int sid;
};

union srs_log_item
{
	struct srs_connect_ip conn_ip;
	struct srs_connect_url conn_url;
	struct srs_trans trans;
	struct srs_disconnect disconn;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//for srs raw log, pair<beg, end>
struct srs_raw_log: public std::pair<char *, char *>
{
	/*raw log type, @see parse_srs_log_item*/
	/*0-other; 1: srs_connect_ip; 2: srs_trans; 3: srs_disconnect; 4: srs_connect_url*/
	int type;
	/*
	 * a srs_raw_log is either a std::pair<char *, char *>, which from mmap call
	 * or a buff(std::string),  which from getline call
	 * */
	std::string buff;
};

typedef srs_raw_log srs_raw_log_t;

/*!
 * log statistics for srs log
 */
struct srs_log_stat
{
	std::unordered_map<int, std::string> urls;	/*sid : url*/
	std::unordered_map<int, uint32_t> ips;		/*sid : ip*/
	std::unordered_map<int, size_t>	obytes;		/*sid : bytes out*/
	std::unordered_map<int, size_t>	ibytes;		/*sid : bytes in*/

	std::vector<srs_raw_log_t> logs;			/*raw logs*/
public:
	size_t obytes_total() const;
	size_t ibytes_total() const;
};

struct srs_domain_stat
{
	std::map<time_group, srs_log_stat> _stats;
	int _site_id;
	int _user_id;
	std::unordered_map<int, std::string>  _sid_log; /*sid : sid_raw_log, @see srs_sid_log*/
};

/*!
 * srs logs by session id
 * srs log format is different from nginx log, which all fields in every log_item
 * while srs log comes with: connect, ..., transform(trans), ..., disconnect
 *
 * so, we first split srs log by sid(srs session id), such like std::unordered_map<int, srs_sid_log>
 * */
class srs_sid_log
{
public:
	int _site_id;
	int _user_id;
	uint32_t _ip;
	size_t _bytes;	/*bytes total by this sid, in this log file*/
	std::vector<srs_raw_log_t> _logs;
	std::string _url;
	std::string _domain;	/*TODO: change to char[]?*/
public:
	srs_sid_log(int sid = 0);
public:
	operator bool() const;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!parse srs log item, currently get <timestamp>, <client_ip>, <time>, <obytes>, <ibytes>
 * format:[timestamp][log_level][srs_pid][srs_sid][errno]<text>
 * @see https://github.com/ossrs/srs/wiki/v1_CN_SrsLog for log_format details
 *
 * log sample:
 * 1.connect:
 * connect_ip: '[2016-11-03 11:27:32.736][trace][21373][105] RTMP client ip=127.0.0.1'
 * connect_url: '[2016-10-20 17:13:08.058][trace][20009][114] connect app, tcUrl=rtmp://192.168.212.164:1935/live, pageUrl=, \
 * swfUrl=, schema=rtmp, vhost=__defaultVhost__, port=1935, app=live, args=null'
 * 2.trans
 * '[2016-11-03 11:31:52.824][trace][21373][105] <- CPB time=240002, obytes=4.09 KB, ibytes=14.29 MB, okbps=0,0,0, ikbps=461,547,0, \
 * mr=0/350, p1stpt=20000, pnt=20000'
 * 3.disconnect:
 * '[2016-11-03 11:34:33.360][warn][21373][110][32] client disconnect peer. ret=1004'
 *
 *@param log_type, 0-other; 1: srs_connect_ip; 2: srs_trans; 3: srs_disconnect; 4: srs_connect_url
 * NOTES:
 * 1.this function is obsolete and may be incorrect
 * */
int parse_srs_log_item(char * buff, srs_log_item& logitem, int& log_type);

/**
 * parse srs cnnect log
 * @param t, 1: ip; 2: url;
 * return 0 on success
 * @see parse_srs_log_item
 * */
int parse_srs_log_item_conn(char const * buff, srs_connect_ip& ip, srs_connect_url & url, int& t);

/**
 * parse srs trans log
 * @note: change srs_raw_log_t.type(@param rlog) if needed
 * @return:  <0 parse failed; =0 parse ok and is trans; 1 parse ok but not trans
 */
int parse_srs_log_item_trans(int sid, srs_raw_log_t & rlog, srs_trans & trans);

/* parse time_stamp, sid from srs_log_header, sample: '[2016-11-03 11:33:16.924][trace][21373][110] '
 * return 0 on success
 * @note: move @param buff to header end
 * @note: add '\0'  to @param buff if needed(time_stamp)
 * @NOTES:
 * 1.move @param buff after parsed
 * 2.this function is obsolete and may be incorrect
 * */
int parse_srs_log_header(char *& buff, time_t & time_stamp, int & sid);

/* parse sid from srs_log_header, sample: '[2016-11-03 11:33:16.924][trace][21373][110] '
 * return sid(>=0), or -1 on failure
 * */
int parse_srs_log_header_sid(char const * buff, char const * end);

/* parse time from srs_log_header, sample: '[2016-11-03 11:33:16.924][trace][21373][110]'
 * return 0 on success
 * */
int parse_srs_log_header_time(char const * buff, char const * end, time_t & t);

/* statistics for srs log
 * @param log_type @see parse_srs_log_item
 * @return 0 on success
 * @NOTES:
 * 2.this function is obsolete and may be incorrect
 * */
int do_srs_log_stats(srs_log_item const& logitem, int log_type,
		std::vector<srs_connect_ip> const& ip_items,
		std::vector<srs_connect_url> const& url_items,
		std::unordered_map<std::string, srs_domain_stat> & logstats);

/*!
 * do srs log statistics
 * this function change srs log from 'by id ' to 'by time_group'
 * @param failed, lines parse failed; @param trans, total lines for trans
 * @see time_group
 * @note: change srs_raw_log_t.type if needed
 * return 0 on success
 */
int do_srs_log_sid_stats(int sid, srs_sid_log & slog, srs_domain_stat & dstat,
		size_t & failed_line, size_t & trans_line);

/* get domain from url, 
 * sample get '127.0.0.1' from 'rtmp://127.0.0.1:1359/'
 * return 0 on success
 * @notes: @param domain big enough
 */
int parse_domain_from_url(char const * url, char * domain);

#endif /*_SRS_LOG_ANALYSIS_H_*/

