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

	char const * output_file_flow;	/*'1' for stdout*/

	char const * output_file_url_popular;
	bool output_file_url_popular_split;				/*split url_popular_table?*/
	char const * output_file_url_popular_format;	/*output url_popular_table filename format*/

	char const * output_file_ip_popular;
	char const * output_file_http_stats;
	char const * output_file_ip_slowfast;
	char const * output_file_cutip_slowfast;

	char const * output_file_ip_source;
	bool output_file_ip_source_split;
	char const * output_file_ip_source_format;

	int device_id;		/*if not provided, find it in devicelist_file by local machine ip*/
	int print_device_id;
	int print_site_user_id;
	int enable_multi_thread;	/*enalbe_multi_thread?*/
	int parse_url_mode;		/*0|1|2, default 2, @see parse_nginx_log_request_uri_url*/
	int show_help;
	int show_version;
	int verbose;
};

/*nginx_log_analysis/option.cpp*/
extern struct nla_options nla_opt;
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

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*_CPP_TEST_OPTIONS_H_*/
