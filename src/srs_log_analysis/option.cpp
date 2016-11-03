#include <cstdlib>	/*atoi*/
#include <cstring>	/*strcmp*/
#include <popt.h>	/*poptOption*/
#include "test_options.h"	/*sla_options*/

static bool sla_options_is_ok(sla_options const& opt);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//sla_options
struct sla_options sla_opt = {
		.log_file = NULL,
		.show_help = 0,
		.show_version = 0,
		.verbose = 0,
};

static poptContext pc = 0;
static struct poptOption sla_popt[] = {
  /* longName, shortName, argInfo, argPtr, value, descrip, argDesc */
	{"log-file",                'l',  POPT_ARG_STRING,   0, 'l', "srs_log_file default: srs.log", 0 },
	{"help",                    'h',    POPT_ARG_NONE,   0, 'h', "print this help", 0 },
	{"version",                 0,    POPT_ARG_NONE,   0, 'V', "print version info and exit", 0},
	{"verbose",                 'v',  POPT_ARG_NONE,   0, 'v', "verbose, print more details", 0 },
	NULL	/*required!!!*/
};

int srs_log_stats_parse_options(int argc, char ** argv)
{
	if(pc)
		poptFreeContext(pc);
	pc = poptGetContext("srs_log_analysis", argc, (const char **)argv, sla_popt, 0);
	for(int opt; (opt = poptGetNextOpt(pc)) != -1; ){
		switch(opt){
		case 'l': sla_opt.log_file = poptGetOptArg(pc); break;
		case 'h': sla_opt.show_help = 1; break;
		case 'V': sla_opt.show_version = 1; break;
		case 'v': sla_opt.verbose = 1; break;
		default:
			break;
		}
	}
	return sla_options_is_ok(sla_opt)? 0 : -1;
}

int test_srs_log_analysis_options_main(int argc, char ** argv)
{
	return srs_log_stats_parse_options(argc, argv);
}

void srs_log_stats_show_help(FILE * stream)
{
	fprintf(stream, "analysis srs log file and print results, build at %s %s\nabout srs:"
			"https://github.com/ossrs/srs/wiki/v2_CN_Home\n"
			, __DATE__, __TIME__);
	poptPrintHelp(pc, stream, 0);
}

void srs_log_stats_show_usage(FILE * stream)
{
	poptPrintUsage(pc, stream, 0);
}

static bool sla_options_is_ok(sla_options const& opt)
{
	/*FIXME: update this function*/
	if(opt.show_help) return true;
	if(opt.show_version) return true;
	return true;
}

void sla_options_fprint(FILE * stream, sla_options const * popt)
{
	if(!popt) return;
	auto& opt = *popt;
	fprintf(stream,
			"%-30s%-20s\n"
			"%-30s%-20d\n" "%-30s%-20d\n"
			"%-30s%-20d\n"
		, "log_file", opt.log_file

		, "show_help", opt.show_help
		, "show_version", opt.show_version

		, "verbose", opt.verbose
	);
}
