/*parse options and arguments*/

#ifndef _CPP_TEST_OPTIONS_H_
#define _CPP_TEST_OPTIONS_H_
#include <stdio.h> /*FILE*/
#ifdef __cplusplus
extern "C"{
#endif

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
	int parse_url_mode;		/*0|1|2, default 2, @see parse_nginx_log_request_uri_url*/

/*for srs_log_analysis*/
	char const * srs_log_file;
	char const * output_file_srs_flow;
/*for output results*/
	int  interval;

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
/**/
	int device_id;		/*if not provided, find it in devicelist_file by local machine ip*/
	int print_device_id;
/**/
	int enable_multi_thread;	/*enalbe_multi_thread?*/
/*version and help*/
	int show_help;
	int show_version;
/*debug*/
	int verbose;

};

/*plcdn_log_analysis/option.cpp*/
void plcdn_la_options_fprint(FILE * stream, plcdn_la_options const * opt);
int plcdn_la_parse_options(int argc, char ** argv);
void plcdn_la_show_help(FILE * stream);
void plcdn_la_show_usage(FILE * stream);

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_CPP_TEST_OPTIONS_H_*/
