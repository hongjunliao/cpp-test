/*!
 * This file is PART of plcdn_log_split
 * cmdline options
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include "test_options.h"	/*plcdn_ls_options*/
#include <popt.h>			/*poptOption*/
#include <stdlib.h>			/*atoi*/
static bool plcdn_ls_options_is_ok(plcdn_ls_options const& opt);

#define DEF_FORMAT_SPLIT_NGINX_LOG   	"${site_id}/${day}"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//plcdn_ls_options
struct plcdn_ls_options plcdn_ls_opt = {
		.nginx_log_file = NULL,
		.siteuidlist_file = "siteuidlist.txt",
		.interval = 300,
		.output_split_nginx_log = NULL,
		.format_split_nginx_log = DEF_FORMAT_SPLIT_NGINX_LOG,
		.show_help = 0,
		.show_version = 0,

		.verbose = 0,
};

static poptContext pc = 0;
static struct poptOption plcdn_ls_popt[] = {
	  /* longName, shortName, argInfo, argPtr, value, descrip, argDesc */
	{"nginx-log-file",          'l',  POPT_ARG_STRING,   0, 'l', "nginx_log_file", 0 },

	{"interval",                'i',  POPT_ARG_INT,      0, 'i', "interval in seconds, default: 300", 0 },

	{"output-split-nginx-log",  'g',  POPT_ARG_STRING,   0, 'g', "output folder for split_nginx_log, disabled if NULL", 0 },
	{"format-split-nginx-log",  'G',  POPT_ARG_STRING,   0, 'G', "filename format for split_nginx_log, default '" DEF_FORMAT_SPLIT_NGINX_LOG "'", 0 },

	{"help",                    'h',    POPT_ARG_NONE,   0, 'h', "print this help", 0 },
	{"version",                   0,    POPT_ARG_NONE,   0, 'V', "print version info and exit", 0},
	{"verbose",                 'v',  POPT_ARG_NONE,     0, 'v', "verbose, print more details", 0 },
	NULL	/*required!!!*/
};

int plcdn_ls_parse_options(int argc, char ** argv)
{
	if(pc)
		poptFreeContext(pc);
	pc = poptGetContext("plcdn_log_split", argc, (const char **)argv, plcdn_ls_popt, 0);
	for(int opt; (opt = poptGetNextOpt(pc)) != -1; ){
		switch(opt){
		case 'l': plcdn_ls_opt.nginx_log_file = poptGetOptArg(pc); break;
		case 'i': plcdn_ls_opt.interval = atoi(poptGetOptArg(pc)); break;

		case 'g': plcdn_ls_opt.output_split_nginx_log = poptGetOptArg(pc); break;
		case 'G': plcdn_ls_opt.format_split_nginx_log = poptGetOptArg(pc); break;

		case 'h': plcdn_ls_opt.show_help = 1; break;
		case 'V': plcdn_ls_opt.show_version = 1; break;
		case 'v': plcdn_ls_opt.verbose = 1; break;
		default:
			break;
		}
	}
	return plcdn_ls_options_is_ok(plcdn_ls_opt)? 0 : -1;
}

void plcdn_ls_show_help(FILE * stream)
{
	fprintf(stream, "split nginx, srs logs into multi-log-files, by site_id and interval. build at %s %s\n"
			, __DATE__, __TIME__);
	poptPrintHelp(pc, stream, 0);
	fprintf(stream, "NOTES:\n  1.about 'filename format'(option --format-*, e.g. --format-split-nginx-log):\n"
			"    ${day}        format YYYYmmDD\n"
			"    ${site_id}    site_id\n"
			"  2.use ulimit(or other command) to increase 'open files' if needed\n"
			"  3.about srs: https://github.com/ossrs/srs/wiki/v2_CN_Home\n"
			"  4.nginx_log_format: $host $remote_addr $request_time_msec $cache_status [$time_local] \"$request_method \
$request_uri $server_protocol\" $status $bytes_sent \
\"$http_referer" "$remote_user" "$http_cookie" "$http_user_agent\" \
$scheme $request_length $upstream_response_time\n"
	);
}

void plcdn_ls_show_usage(FILE * stream)
{
	poptPrintUsage(pc, stream, 0);
}

static bool plcdn_ls_options_is_ok(plcdn_ls_options const& opt)
{
	/*FIXME: update this function*/
	if(opt.show_help) return true;
	if(opt.show_version) return true;
	return true;
}

void plcdn_ls_options_fprint(FILE * stream, plcdn_ls_options const * popt)
{
	if(!popt) return;
	auto& opt = *popt;
	fprintf(stream,
			"%-34s%-20s\n"
			"%-34s%-20d\n"
			"%-34s%-20s\n" "%-34s%-20s\n"
			"%-34s%-20d\n" "%-34s%-20d\n"
			"%-34s%-20d\n"
		, "nginx_log_file", opt.nginx_log_file
		, "interval", opt.interval

		, "output_split_nginx_log", opt.output_split_nginx_log
		, "format_split_nginx_log", opt.format_split_nginx_log

		, "show_help", opt.show_help
		, "show_version", opt.show_version

		, "verbose", opt.verbose
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
int test_plcdn_log_split_options_main(int argc, char ** argv)
{
	return plcdn_ls_parse_options(argc, argv);
}

