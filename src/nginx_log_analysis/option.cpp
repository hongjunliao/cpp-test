#include <cstdlib>	/*atoi*/
#include <cstring>	/*strcmp*/
#include <popt.h>	/*poptOption*/
#include "test_options.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//nla_options

static bool nla_options_is_ok(nla_options const& opt);

nla_options nla_opt = {
		.log_file = "nginx.log",
		.interval = 300,
		.devicelist_file = "devicelist.txt",
		.siteuidlist_file = "siteuidlist.txt",
		.ipmap_file = "iplocation.bin",

		.output_file_flow = NULL,
		.output_file_url_popular = NULL,
		.output_file_url_popular_split = false,
		.output_file_ip_popular = NULL,
		.output_file_http_stats = NULL,
		.output_file_ip_slowfast = NULL,
		.output_file_cutip_slowfast = NULL,
		.output_file_ip_source = NULL,

		.flow = 0,
		.url_popular = 0,
		.ip_popular = 0,
		.http_stats = 0,
		.cutip_slowfast = 0,
		.ip_source = 0,

		.device_id = 0,
		.print_device_id = 0,
		.print_site_user_id = 0,
		.enable_multi_thread = 0,
		.show_help = 0,
		.show_version = 0,
		.verbose = 0,
};
static poptContext pc = 0;
static struct poptOption nla_popt[] = {
  /* longName, shortName, argInfo, argPtr, value, descrip, argDesc */
	{"log-file",                'l',  POPT_ARG_STRING,   0, 'l', "nginx_log_file default: nginx.log", 0 },
	{"interval",                'i',  POPT_ARG_INT,      0, 'i', "interval in seconds, default: 300", 0 },
	{"device-list-file",        'd',  POPT_ARG_STRING,   0, 'd', "devicelist_file default: devicelist.txt", 0 },
	{"siteuid-list-file",       's',  POPT_ARG_STRING,   0, 's', "siteuidlist_file default: siteuidlist.txt", 0 },
	{"ipmap-file",              'm',  POPT_ARG_STRING,   0, 'm', "ipmap_file default: iplocation.bin", 0 },
	{"output-file-flow",        'o',  POPT_ARG_STRING,   0, 'o', "output_file, flow table, 1 for to stdout", 0 },
	{"output-file-url-popular", 'u',  POPT_ARG_STRING,   0, 'u', "output_file, url_popular table, 1 for to stdout", 0 },
	{"output-file-ip-popular",  'p',  POPT_ARG_STRING,   0, 'p', "output_file, ip_popular table, 1 for to stdout", 0 },
	{"output-file-http-stats",  't',  POPT_ARG_STRING,   0, 't', "output_file, http_stats table, 1 for to stdout", 0 },
	{"output-file-ip-slowfast", 'w',  POPT_ARG_STRING,   0, 'w', "output_file, ip_slowfast table, 1 for to stdout", 0 },
	{"output-file-cutip-slowfast"  , 'f',  POPT_ARG_STRING,   0, 'f', "output_file, cutip_slowfast table, 1 for to stdout", 0 },
	{"output-file-ip-source",   'r',  POPT_ARG_STRING,   0, 'r', "output_file, ip_source table, 1 for to stdout", 0 },
	{"device-id",               'e',  POPT_ARG_INT,     0,  'e', "device_id integer(> 0)", 0 },
	{"print-divice-id",         'c',  POPT_ARG_NONE,   0,   'c', "print device_id and exit", 0 },
	{"print-site-user-id",      'n',  POPT_ARG_NONE,   0,   'n', "print site_user_id and exit, output format:<site_id> <user_id>", 0 },
	{"enable-multi-thread",       0,  POPT_ARG_NONE,   0,   'a', "enable_multi_thread", 0 },
	{"help",                    'h',    POPT_ARG_NONE,   0, 'h', "print this help", 0 },
	{"version",                   0,    POPT_ARG_NONE,   0, 'V', "print version info and exit", 0},
	{"verbose",                 'v',  POPT_ARG_NONE,   0, 'v', "verbose, print more details", 0 },
	NULL	/*required!!!*/
};

int nginx_log_stats_parse_options(int argc, char ** argv)
{
	if(pc)
		poptFreeContext(pc);
	pc = poptGetContext("nginx_log_analysis", argc, (const char **)argv, nla_popt, 0);
	for(int opt; (opt = poptGetNextOpt(pc)) != -1; ){
		switch(opt){
		case 'l': nla_opt.log_file = poptGetOptArg(pc); break;
		case 'i': nla_opt.interval = atoi(poptGetOptArg(pc)); break;
		case 'e': nla_opt.device_id = atoi(poptGetOptArg(pc)); break;
		case 'd': nla_opt.devicelist_file = poptGetOptArg(pc); break;
		case 's': nla_opt.siteuidlist_file = poptGetOptArg(pc); break;
		case 'm': nla_opt.ipmap_file = poptGetOptArg(pc); break;
		case 'o': { nla_opt.flow = 1; nla_opt.output_file_flow = poptGetOptArg(pc); } break;
		case 'u': {
			nla_opt.url_popular = 1;
			auto str = poptGetOptArg(pc);
			char * p = strchr(str, ',');
			if(p){
				*p = '\0';
				//FIXME: regex?
				nla_opt.output_file_url_popular_split = (strcmp("split=1", ++p) == 0);
			}
			nla_opt.output_file_url_popular = str;
		}
		break;
		case 'p': { nla_opt.ip_popular = 1; nla_opt.output_file_ip_popular = poptGetOptArg(pc); } break;
		case 't': { nla_opt.http_stats = 1; nla_opt.output_file_http_stats = poptGetOptArg(pc); } break;
		case 'w': { nla_opt.output_file_ip_slowfast = poptGetOptArg(pc); } break;
		case 'f': { nla_opt.cutip_slowfast = 1; nla_opt.output_file_cutip_slowfast = poptGetOptArg(pc); } break;
		case 'r': { nla_opt.ip_source = 1; nla_opt.output_file_ip_source = poptGetOptArg(pc); } break;
		case 'c': nla_opt.print_device_id = 1; break;
		case 'n': nla_opt.print_site_user_id = 1; break;
		case 'a': nla_opt.enable_multi_thread = 1; break;
		case 'h': nla_opt.show_help = 1; break;
		case 'V': nla_opt.show_version = 1; break;
		case 'v': nla_opt.verbose = 1; break;
		default:
			break;
		}
	}
	return nla_options_is_ok(nla_opt)? 0 : -1;
}

int test_nginx_log_analysis_options_main(int argc, char ** argv)
{
	return nginx_log_stats_parse_options(argc, argv);
}

void nginx_log_stats_show_help(FILE * stream)
{
	fprintf(stream, "analysis nginx log file and print results, build at %s %s\n", __DATE__, __TIME__);
	poptPrintHelp(pc, stream, 0);
	fprintf(stream, "NOTES:\n  1.append option 'split=1'to split output to multi-files by time interval(fmt:YYYYmmDDHHMM), currently support:\n"
			"\t-u url_popular,split=1('url_popular' treated as a folder now; comma separated) \n"
			"\t-w ip_slowfast,split=1\n"
			);
}

void nginx_log_stats_show_usage(FILE * stream)
{
	poptPrintUsage(pc, stream, 0);
}

static bool nla_options_is_ok(nla_options const& opt)
{
	/*FIXME: update this function*/
	if(opt.show_help) return true;
	if(opt.show_version) return true;
	if(opt.print_device_id) return true;
	bool result = (opt.log_file && opt.interval > 0
			&& opt.devicelist_file  && opt.siteuidlist_file
			&& (opt.device_id >=  0)
			);
	return result;
}

void nla_options_fprint(FILE * stream, nla_options const * popt)
{
	if(!popt) return;
	auto& opt = *popt;
	fprintf(stream,
			"%-30s%-20s" "\n%-30s%-20d" "\n%-30s%-20s\n" "%-30s%-20s\n" "%-30s%-20s\n"
			"%-30s%-20d\n" "%-30s%-20d\n" "%-30s%-20d\n" "%-30s%-20d\n"
			"%-30s%-20s\n" "%-30s%-20s\n" "%-30s%-20d\n"
			"%-30s%-20s\n" "%-30s%-20s\n" "%-30s%-20s\n" "%-30s%-20s\n" "%-30s%-20s\n"
			"%-30s%-20d\n" "%-30s%-20d\n" "%-30s%-20d\n" "%-30s%-20d\n" "%-30s%-20d\n"
			"%-30s%-20d\n"
		, "log_file", opt.log_file
		, "interval", opt.interval
		, "devicelist_file", opt.devicelist_file
		, "siteuidlist_file", opt.siteuidlist_file
		, "ipmap_file", opt.ipmap_file

		, "flow", opt.flow
		, "url_popular", opt.url_popular
		, "ip_popular", opt.ip_popular
		, "http_stats", opt.http_stats

		, "output_file_flow", opt.output_file_flow
		, "output_file_url_popular", opt.output_file_url_popular
		, "output_file_url_popular_split", opt.output_file_url_popular_split

		, "output_file_ip_popular", opt.output_file_ip_popular
		, "output_file_http_stats", opt.output_file_http_stats
		, "output_file_ip_slowfast", opt.output_file_ip_slowfast
		, "output_file_cutip_slowfast", opt.output_file_cutip_slowfast
		, "output_file_ip_source", opt.output_file_ip_source

		, "device_id", opt.device_id
		, "print_device_id", opt.print_device_id
		, "print_site_user_id", opt.print_site_user_id
		, "show_help", opt.show_help
		, "show_version", opt.show_version

		, "verbose", opt.verbose
	);
}

