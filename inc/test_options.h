/*parse options and arguments*/

#ifndef _CPP_TEST_OPTIONS_H_
#define _CPP_TEST_OPTIONS_H_
#include <stdio.h> /*FILE*/
#include <time.h> /*time_t*/
#ifdef __cplusplus
extern "C"{
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*nla: nginx_log_analysis*/
struct nla_options
{
	char const * log_file;
	int  interval;
	char const * devicelist_file;
	char const * siteuidlist_file;
	char const * ipmap_file;

	char const * output_file_flow;	/*flow_table output dir*/
	char const * format_flow;		/*output flow_table filename format*/

	char const * output_file_url_popular;
	char const * format_url_popular;

	char const * output_file_ip_popular;
	char const * format_ip_popular;
	int min_ip_popular, max_ip_popular; /*for filter output, minimal access_count for all ip*/

	char const * output_file_http_stats;
	char const * format_http_stats;

	char const * output_file_ip_slowfast;
	char const * format_ip_slowfast;

	char const * output_file_cutip_slowfast;
	char const * format_cutip_slowfast;

	char const * output_file_ip_source;
	char const * format_ip_source;

	int device_id;		/*if not provided, find it in devicelist_file by local machine ip*/
	int print_device_id;
	int enable_multi_thread;	/*enalbe_multi_thread?*/
	int parse_url_mode;		/*0|1|2, default 2, @see parse_nginx_log_request_uri_url*/
	int show_help;
	int show_version;
	int verbose;
};

/*nginx_log_analysis/option.cpp*/
void nla_options_fprint(FILE * stream, nla_options const * opt);
int nginx_log_stats_parse_options(int argc, char ** argv);
void nginx_log_stats_show_help(FILE * stream);
void nginx_log_stats_show_usage(FILE * stream);

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*sla: srs_log_analysis*/
struct sla_options
{
	char const * log_file;
	int show_help;
	int show_version;
	int verbose;
};

/*srs_log_analysis/option.cpp*/
void sla_options_fprint(FILE * stream, sla_options const * opt);
int srs_log_stats_parse_options(int argc, char ** argv);
void srs_log_stats_show_help(FILE * stream);
void srs_log_stats_show_usage(FILE * stream);

/*plcdn_la: plcdn_log_analysis*/
struct plcdn_la_options
{
/*for ngnx_log_analysis*/
	char const * nginx_log_file;
	char const * devicelist_file;
	char const * siteuidlist_file;
	char const * ipmap_file;
	int parse_url_mode;							/*0|1|2, default 2, @see parse_nginx_log_request_uri_url*/
/*time range for log*/
	time_t begin_time, end_time; 				/* time range for log, format [begin_time, end_time), disabled if 0*/

	int no_merge_datetime;					/* default off, if set, don't merge rows in before print tables where datetime same */
/*for srs_log_analysis*/
	char const * srs_log_file;
	int srs_calc_flow_mode;						/* for srs, mode for calculate flow, 0: use obytes/ibytes, 1: use okbps/ikbps */
	char const * output_srs_flow;
	char const * format_srs_flow;
	char const * srs_sid_dir;					/*dir for srs_sid, @see srs_log_analysis/split_log.cpp/sync_srs_sids_dir*/
	char const * output_split_srs_log_by_sid;	/*folder for split srs log by sid*/
/*for output results*/
	int  interval;

	char const * output_nginx_flow;				/*flow_table output dir*/
	char const * format_nginx_flow;				/*output flow_table filename format*/

	char const * output_file_url_popular;
	char const * format_url_popular;

	char const * output_file_ip_popular;
	char const * format_ip_popular;
	int min_ip_popular, max_ip_popular; 		/*for filter output, minimal access_count for all ip*/

	char const * output_file_http_stats;
	char const * format_http_stats;

	char const * output_file_ip_slowfast;
	char const * format_ip_slowfast;

	char const * output_file_cutip_slowfast;
	char const * format_cutip_slowfast;

	char const * output_file_ip_source;
	char const * format_ip_source;
/*split log by domain*/
	char const * output_split_nginx_log;
	char const * format_split_nginx_log;

	char const * output_split_srs_log;
	char const * format_split_srs_log;
/**/
	int device_id;		/*if not provided, find it in devicelist_file by local machine ip*/
	int print_device_id;
/**/
	int enable_multi_thread;	/*enalbe_multi_thread?*/
/* work mode, 0: analysis mode: analysis log files
 * 1: merge_srs_flow */
	int work_mode;
/* append/merge table */
	int append_flow_nginx;		/* append other(currently srs) flow to nginx output, 0 or 1, default 0 */
/*version and help*/
	int show_help;
	int show_version;
/*debug, 0: close, >=1: verbose*/
	int verbose;

};

/*plcdn_log_analysis/option.cpp*/
void plcdn_la_options_fprint(FILE * stream, plcdn_la_options const * opt);
int plcdn_la_parse_options(int argc, char ** argv);
void plcdn_la_show_help(FILE * stream);
void plcdn_la_show_usage(FILE * stream);

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*plcdn_ls: plcdn_log_split*/
struct plcdn_ls_options
{
	char const * nginx_log_file;
	char const * siteuidlist_file;
	int  interval;

	char const * output_split_nginx_log;
	char const * format_split_nginx_log;
/*version and help*/
	int show_help;
	int show_version;
/*debug*/
	int verbose;
};

/*plcdn_log_split/option.cpp*/
void plcdn_ls_options_fprint(FILE * stream, plcdn_ls_options const * opt);
int plcdn_ls_parse_options(int argc, char ** argv);
void plcdn_ls_show_help(FILE * stream);
void plcdn_ls_show_usage(FILE * stream);

/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_CPP_TEST_OPTIONS_H_*/
