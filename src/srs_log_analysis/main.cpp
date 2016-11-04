/*!
 * This file is PART of srs_log_analysis
 * parse srs logs and print results
 * @author hongjun.liao <docici@126.com>
 * @date 2016/9
 * 1.about srs: https://github.com/ossrs/srs/tree/2.0release
 * 2.about srs_log: https://github.com/ossrs/srs/wiki/v1_CN_SrsLog
 *
 * @NOTE:
   1.FIXME:std::regex FAILED, gcc-4.8 c++11 NOT fully support, use boost::regex instead
 * 2.g++ link options: -lboost_regex
 */

#include <stdio.h>
#include <cstring>
#include <vector>				/*std::vector*/
//#include <regex>
#include <boost/regex.hpp> 		/*regex_search*/
#include "bd_test.h"			/*test_srs_log_stats_main*/
#include "test_options.h" 		/*sla_options*/

#include "net_util.h"			/*netutil_get_ip_from_str*/
#include "srs_log_analysis.h"	/*srs_log_item, ...*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!parse srs log item, currently get <timestamp>, <client_ip>, <time>, <obytes>, <ibytes>
 * format:[timestamp][log_level][srs_pid][srs_sid][errno]<text>
 * @see https://github.com/ossrs/srs/wiki/v1_CN_SrsLog for log_format details
 *
 * log sample:
 * 1.connect:
 * [2016-11-03 11:27:32.736][trace][21373][105] RTMP client ip=127.0.0.1
 * 2.trans
 * [2016-11-03 11:31:52.824][trace][21373][105] <- CPB time=240002, obytes=4.09 KB, ibytes=14.29 MB, okbps=0,0,0, ikbps=461,547,0, \
 * mr=0/350, p1stpt=20000, pnt=20000
 * 3.disconnect:
 * [2016-11-03 11:34:33.360][warn][21373][110][32] client disconnect peer. ret=1004
 *
 *@param log_type, 0-other; 1-connect; 2-trans; 3-disconect
 * */
static int parse_srs_log_item(char * buff, srs_log_item& logitem, int& log_type);
/*srs_log_analysis/print_table.cpp*/
extern void fprint_srs_log_stats(FILE * stream, std::vector<srs_trans> const& trans_stats,
		std::vector<srs_connect> const& cstats);
/*GLOBAL vars*/
extern struct sla_options sla_opt;	/*test_options.cpp*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int parse_srs_log_item(char * buff, srs_log_item& logitem, int& log_type)
{
	/*parse time_stamp, sid from '[2016-11-03 11:33:16.924][trace][21373][110] '*/
	time_t time_tamp = 0;
	int sid;
	char * lbuff;
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
				time_tamp = mktime(&my_tm);
			}
			else if(index == 3){
				/*FIXME: sid CAN BE 0?*/
				sid = atoi(p + 1);
				lbuff = q + 1;
				break;
			}
			p = q + 1;
			++index;
			is_arg_start = false;
		}
	}

	static auto s1 = "RTMP client ip=([0-9.]+)";
	static auto s2 = "(?:<- CPB|-> PLA) time=[0-9]+, (?:msgs=[0-9]+, )?obytes=([0-9]+), ibytes=([0-9]+),"
				 " okbps=([0-9]+),[0-9]+,[0-9]+, ikbps=([0-9]+),[0-9]+,[0-9]+";
	static auto s3 = "client disconnect peer\\. ret=[0-9]+";
	static boost::regex r1{s1}, r2{s2}, r3{s3};

	boost::cmatch m;
	if(boost::regex_search(lbuff, m, r1)) {
		log_type = 1;
		memset(&logitem.conn, 0, sizeof(logitem.conn));
		logitem.conn.time_stamp = time_tamp;
		logitem.conn.sid = sid;
		logitem.conn.ip = netutil_get_ip_from_str(m[1].str().c_str());
	}
	else if(boost::regex_search(lbuff, m, r2)) {
		log_type = 2;
		memset(&logitem.trans, 0, sizeof(logitem.trans));
		logitem.trans.time_stamp = time_tamp;
		logitem.trans.sid = sid;
		char * end;
		logitem.trans.obytes = strtoul(m[1].str().c_str(), &end, 10);
		logitem.trans.ibytes = strtoul(m[2].str().c_str(), &end, 10);
		logitem.trans.okbps = strtoul(m[3].str().c_str(), &end, 10);
		logitem.trans.ikbps = strtoul(m[4].str().c_str(), &end, 10);
	}
	else if(boost::regex_search(lbuff, m, r3)) {
		log_type = 3;
	}
	else{
		log_type = 0;
		return 0;
	}
	return time_tamp == 0? -1 : 0;
}

int test_srs_log_stats_main(int argc, char ** argv)
{
	auto satus = srs_log_stats_parse_options(argc, argv);
	if(satus != 0 || sla_opt.show_help){
		sla_opt.show_help? srs_log_stats_show_help(stdout) :
				srs_log_stats_show_usage(stdout);
		return 0;
	}
	if(sla_opt.verbose)
		sla_options_fprint(stdout, &sla_opt);

	auto log_file = sla_opt.log_file? fopen(sla_opt.log_file, "r") : stdin;
	if(!log_file) {
		fprintf(stderr, "%s: fopen file %s failed\n", __FUNCTION__, sla_opt.log_file);
		return 1;
	}
	size_t linecount = 0, failed_count = 0, skip_count = 0;
	char data[8192];
	char const * result = 0;
	std::vector<srs_trans> tstats;
	std::vector<srs_connect> cstats;
	while((result = fgets(data, sizeof(data), log_file)) != NULL){
		++linecount;
		if(sla_opt.verbose && linecount % 1000 == 0)
			fprintf(stdout, "\r%s: processing %8ld line ...", __FUNCTION__, linecount);
		auto len = strlen(result);
		if(result[len - 1] != '\n'){
			fprintf(stderr, "\n%s: WARNING, length > %zu bytes, skip:\n%s\n", __FUNCTION__, sizeof(data), data);
			continue;
		}
		data[len - 1] = '\0';
		srs_log_item logitem;
		int log_type;
		auto result = parse_srs_log_item(data, logitem, log_type);
		if(result != 0) {
			++failed_count;
			continue;
		}

		if(log_type == 2)
			tstats.push_back(logitem.trans);
		else if(log_type == 1)
			cstats.push_back(logitem.conn);
		else if(log_type == 0)
			++skip_count;
	}
	if(sla_opt.verbose)
		fprintf(stdout, "\r%s: processed, total_line: %zu, failed=%zu, skip=%zu\n", __FUNCTION__
				, linecount, failed_count, skip_count);

	fprint_srs_log_stats(stdout, tstats, cstats);
	return 0;
}
