/*parse options and arguments*/

#ifndef PLCDN_LA_OPTIONS_H_
#define PLCDN_LA_OPTIONS_H_
#include <stdio.h> /*FILE*/
#include <time.h> /*time_t*/
#ifdef __cplusplus
extern "C"{
#endif

/* format for nginx log, all are indexes for that field;
 * -1: this field NOT found, >=0: index for this field */
struct ngx_log_format {
	int host;
	int remote_addr;
	int request_time_msec;
	int upstream_cache_status;
	int time_local;
	int request_method;
	int request_uri;
	int server_protocol;
	int status;
	int bytes_sent;
	int http_referer;
	int remote_user;
	int http_user_agent;
	int scheme;
	int request_length;
	int upstream_response_time;
	int start_response_time_msec;
	int body_bytes_sent;
	int http_x_forwarded_for;
	int connection;
	int server_addr;

	int site_id;
	int upstream_addr;
	int log_cache_uri;

	/* index for sub items in nginx log format, e.g. format:
	 *
	 * '$host $remote_addr $request_time_msec $upstream_cache_status [$time_local] \
	 * "$request_method $request_uri $server_protocol" $status $bytes_sent "$http_referer" "$remote_user" - "$http_user_agent" \
	 * $scheme $request_length $upstream_response_time $start_response_time_msec $body_bytes_sent "$http_x_forwarded_for" \
	 * "$connection" "$server_addr"'
	 *
	 * has 1 sub_item "$request_method $request_uri $server_protocol", it's index is 5, this sub_item should be split to 3 items
	 *
     */
	int sub_items[32];
	int n_sub;
};

/*plcdn_la: plcdn_log_analysis*/
struct plcdn_la_options
{
/*for ngnx_log_analysis*/
	char const * nginx_log_file;
	char const * nginx_rotate_dir;				/* for work_mode == 2 */
	int nginx_rotate_time;						/* for work_mode == 2, rotate time in seconds */
	char const * devicelist_file;
	char const * siteuidlist_file;
	char const * ipmap_file;
	int parse_url_mode;							/*0|1|2, default 2, @see parse_nginx_log_request_uri_url*/
	/* values for nginx hit, format A|B|C,
	 * current, hit: STALE|UPDATING|REVALIDATED|HIT,  miss: MISS|BYPASS|EXPIRED (and other values)*/
	char const * nginx_hit;
	/* nginx http_user_agent for pc, current:
	 * Linux|Windows|Macintosh */
	char const * nginx_ua_pc;
/*time range for log*/
	time_t begin_time, end_time; 				/* time range for log, format [begin_time, end_time), disabled if 0*/

	int no_merge_datetime;					    /* default off, if set, don't merge rows in before print tables where datetime same */
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

	char const *output_file_url_key;
	char const *format_file_url_key;

	char const *output_file_http_ref_ua;        /* $http_referer and $http_user_agent */
	char const *format_file_http_ref_ua;

	char const * local_url_key;
/*split log by domain*/
	char const * output_split_nginx_log;
	char const * format_split_nginx_log;

	char const * output_split_srs_log;
	char const * format_split_srs_log;
/* for transform log */
	int nginx_trans_log;	/* transform nginx log? default 0(disabled) */
/**/
	int device_id;		/*if not provided, find it in devicelist_file by local machine ip*/
	int print_device_id;
/**/
	int enable_multi_thread;	/*enalbe_multi_thread?*/
/* work mode, 0: analysis mode: analysis log files; 1: merge_srs_flow; 2: rotate */
	int work_mode;
/* append/merge table */
	int append_flow_nginx;		/* append other(currently srs) flow to nginx output, 0 or 1, default 0 */
/*version and help*/
	int show_help;
	int show_version;
/* log file */
	char const * log_file;
/* config file, .plcdn_la */
	char const * config_file;
/* ngnx log format */
	struct ngx_log_format ngx_logfmt;
/*debug, 0: close, >=1: verbose*/
	int verbose;

};

/*plcdn_log_analysis/option.cpp*/
void plcdn_la_options_fprint(FILE * stream, plcdn_la_options const * opt);
int plcdn_la_parse_options(int argc, char ** argv);
void plcdn_la_show_help(FILE * stream);
void plcdn_la_show_usage(FILE * stream);

/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif	//__cplusplus

#endif /*PLCDN_LA_OPTIONS_H_*/
