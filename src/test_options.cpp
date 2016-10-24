#include <cstdlib>	/*atoi*/
#include <popt.h>	/*poptOption*/
#include "test_options.h"

static bool nla_options_is_ok(nla_options const& opt);

nla_options nla_opt = { 0 };
static poptContext pc;
static struct poptOption nla_popt[] = {
  /* longName, shortName, argInfo, argPtr, value, descrip, argDesc */
	{"log-file",             'l',  POPT_ARG_STRING,   0, 'l', "nginx_log_file e.g. access.log", 0 },
	{"interval",             'i',  POPT_ARG_INT,     0, 'i', "interval in seconds, e.g. 300", 0 },
	{"device-list-file",     'd',  POPT_ARG_STRING,   0, 'd', "devicelist_file e.g. devicelist.txt", 0 },
	{"siteuid-list-file",    's',  POPT_ARG_STRING,   0, 's', "siteuidlist_file e.g. siteuidlist.txt", 0 },
	{"output-file",          'o',  POPT_ARG_STRING,   0, 'o', "output_file e.g. /tmp/nginx.out", 0 },
	{"print-divice-id",      'c',  POPT_ARG_NONE,   0, 'c', "print device_id and exit", 0 },
	{"help",                 0,  POPT_ARG_NONE,   0, 'h', "print this help", 0 },
	{"version",              0,  POPT_ARG_NONE,   0, 'V', "print version info and exit", 0},
	{"verbose",              'v', POPT_ARG_NONE,   0, 'v', "verbose, print more details", 0 },
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
		case 'd': nla_opt.devicelist_file = poptGetOptArg(pc); break;
		case 's': nla_opt.siteuidlist_file = poptGetOptArg(pc); break;
		case 'o': nla_opt.output_file = poptGetOptArg(pc); break;
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
	fprintf(stream, "analysis nginx log file and print results\n");
	poptPrintHelp(pc, stream, 0);
}

void nginx_log_stats_show_usage(FILE * stream)
{
	poptPrintUsage(pc, stream, 0);
}

static bool nla_options_is_ok(nla_options const& opt)
{
	if(opt.show_help) return true;
	if(opt.show_version) return true;
	if(opt.show_device_id) return true;
	bool result = (opt.log_file && opt.interval > 0
			&& opt.devicelist_file  && opt.siteuidlist_file
			&& opt.output_file);
	return result;
}

void nla_options_fprint(FILE * stream, nla_options const * popt)
{
	if(!popt) return;
	auto& opt = *popt;
	fprintf(stream, "%-30s%-20s\n%-30s%-20d\n%-30s%-20s\n%-30s%-20s\n"
			"%-30s%-20s\n%-30s%-20d\n%-30s%-20d\n%-30s%-20d\n%-30s%-20d\n"
		, "log_file", opt.log_file
		, "interval", opt.interval
		, "devicelist_file", opt.devicelist_file
		, "siteuidlist_file", opt.siteuidlist_file
		, "output_file", opt.output_file
		, "show_device_id", opt.show_device_id
		, "show_help", opt.show_help
		, "show_version", opt.show_version
		, "verbose", opt.verbose
	);
}
