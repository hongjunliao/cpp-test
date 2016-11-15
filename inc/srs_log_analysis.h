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
};

/*srs log_type transform*/
struct srs_trans
{
	time_t time_stamp;
	int sid;
	size_t ibytes;
	size_t obytes;
	size_t ikbps;
	size_t okbps;
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
struct srs_log_stat
{
	int sid;
	/*FIXME: change to std::string?*/
	char const * url;
	uint32_t ip;

	size_t obytes, ibytes;
};

struct srs_domain_stat
{
	std::map<time_group, srs_log_stat> _stats;
	int _site_id;
	int _user_id;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!parse srs log item, currently get <timestamp>, <client_ip>, <time>, <obytes>, <ibytes>
 * format:[timestamp][log_level][srs_pid][srs_sid][errno]<text>
 * @see https://github.com/ossrs/srs/wiki/v1_CN_SrsLog for log_format details
 *
 * log sample:
 * 1.connect:
 * connect_ip: [2016-11-03 11:27:32.736][trace][21373][105] RTMP client ip=127.0.0.1
 * connect_url: [2016-10-20 17:13:08.058][trace][20009][114] connect app, tcUrl=rtmp://192.168.212.164:1935/live, pageUrl=, \
 * swfUrl=, schema=rtmp, vhost=__defaultVhost__, port=1935, app=live, args=null
 * 2.trans
 * [2016-11-03 11:31:52.824][trace][21373][105] <- CPB time=240002, obytes=4.09 KB, ibytes=14.29 MB, okbps=0,0,0, ikbps=461,547,0, \
 * mr=0/350, p1stpt=20000, pnt=20000
 * 3.disconnect:
 * [2016-11-03 11:34:33.360][warn][21373][110][32] client disconnect peer. ret=1004
 *
 *@param log_type, 0-other; 1: srs_connect_ip; 2: srs_trans; 3: srs_disconnect; 4: srs_connect_url
 * */
int parse_srs_log_item(char * buff, srs_log_item& logitem, int& log_type);

/* parse time_stamp, sid from srs_log_header, sample: '[2016-11-03 11:33:16.924][trace][21373][110] '
 * return 0 on success
 * @NOTES: move @param buff after parsed
 * */
int parse_srs_log_header(char *& buff, time_t & time_stamp, int & sid);

/* statistics for srs log
 * @param log_type @see parse_srs_log_item
 * @return 0 on success
 * */
int do_srs_log_stats(srs_log_item const& logitem, int log_type,
		std::vector<srs_connect_ip> const& ip_items,
		std::vector<srs_connect_url> const& url_items,
		std::unordered_map<std::string, srs_domain_stat> & logstats);

#endif /*_SRS_LOG_ANALYSIS_H_*/

