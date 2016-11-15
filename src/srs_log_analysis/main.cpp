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

#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
#include <stdio.h>
#include <cstring>
#include <vector>				/*std::vector*/
#include <unordered_map> 	/*std::unordered_map*/
#include "bd_test.h"			/*test_srs_log_stats_main*/
#include "test_options.h" 		/*sla_options*/

#include "srs_log_analysis.h"	/*srs_log_item, ...*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*srs_log_analysis/print_table.cpp*/
extern void fprint_srs_log_stats(FILE * stream, std::unordered_map<std::string, srs_domain_stat> const& srs_stats);
/*GLOBAL vars*/
extern struct sla_options sla_opt;	/*test_options.cpp*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int parse_srs_log(FILE * f, std::unordered_map<std::string, srs_domain_stat> & logstats)
{
	struct stat logfile_stat;
	if(fstat(fileno(f), &logfile_stat) < 0){
		fprintf(stderr, "%s: fstat() failed for %s\n", __FUNCTION__, "srs_log_file");
		return 1;
	}
	/*FIXME: PAGE_SIZE?*/
	char  * start_p = (char *)mmap(NULL, logfile_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f), 0);
	if(!start_p || start_p == MAP_FAILED){
		fprintf(stderr, "%s: mmap() failed for %s\n", __FUNCTION__, "srs_log_file");
		return 1;
	}

	size_t linecount = 0, failed_count = 0, skip_count = 0;
	std::vector<srs_connect_ip> ip_items;
	std::vector<srs_connect_url> url_items;

	srs_log_item logitem;
	for(char * p = start_p, * q = p; q != start_p + logfile_stat.st_size; ++q){
		if(*q == '\n'){
			*q = '\0';
			++linecount;

			int log_type;
			/*FIXME: srs_connect_ip and srs_connect_url always comes before srs_trans?*/
			auto status = parse_srs_log_item(p, logitem, log_type);
			if(status == 0) {
				switch(log_type){
				case 2:
					status = do_srs_log_stats(logitem, log_type, ip_items, url_items, logstats);
					if(status != 0)
						++failed_count;
					break;
				case 1:
					ip_items.push_back(logitem.conn_ip);
					break;
				case 4:
					url_items.push_back(logitem.conn_url);
					break;
				case 0:
					++skip_count;
					break;
				default:
					break;
				}
			}
			else
				++failed_count;
			p = q + 1;
		}
	}
	if(sla_opt.verbose)
		fprintf(stdout, "%s: processed, total_line: %zu, failed=%zu, skip=%zu\n", __FUNCTION__
				, linecount, failed_count, skip_count);
	return 0;
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

	std::unordered_map<std::string, srs_domain_stat> srs_stats;
	parse_srs_log(log_file, srs_stats);

	fprint_srs_log_stats(stdout, srs_stats);
	return 0;
}
