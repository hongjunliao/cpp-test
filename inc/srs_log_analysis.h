/*!
 * This file is PART of srs_log_analysis
 * data structs
 */
#ifndef _SRS_LOG_ANALYSIS_H_
#define _SRS_LOG_ANALYSIS_H_
#include <time.h>	/*time_t*/
#include <stdint.h>	/*uint32_t*/
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
	int sid;
	uint32_t ip;
};

struct srs_trans
{
	time_t time_stamp;
	int sid;
	size_t ibytes;
	size_t obytes;
	size_t ikbps;
	size_t okbps;
};

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

#endif /*_SRS_LOG_ANALYSIS_H_*/

