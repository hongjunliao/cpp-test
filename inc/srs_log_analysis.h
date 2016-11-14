/*!
 * This file is PART of srs_log_analysis
 * data structs
 */
#ifndef _SRS_LOG_ANALYSIS_H_
#define _SRS_LOG_ANALYSIS_H_
#include "nginx_log_analysis.h"	/*time_group*/
#include <time.h>	/*time_t*/
#include <stdint.h>	/*uint32_t*/
#include <vector>	/*std::vector*/
/*declares*/
struct srs_connect;
struct srs_trans;
struct srs_disconnect;
union srs_log_item;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*srs log_type*/
struct srs_connect
{
	time_t time_stamp;
	/*FIXME: change to std::string?*/
	char const * url;
	int sid;
	uint32_t ip;
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
	struct srs_connect conn;
	struct srs_trans trans;
	struct srs_disconnect disconn;
};

class trans_stat
{
	srs_connect conn;
	time_t start, end;
};
/*/*srs log parse output resutls*/
struct srs_log_stats
{
	size_t linecount, failed_count, skip_count;
	std::vector<srs_trans> tstats;
	std::vector<srs_connect> cstats;
	std::map<time_group, trans_stat> _trans_stats;
};

#endif /*_SRS_LOG_ANALYSIS_H_*/

