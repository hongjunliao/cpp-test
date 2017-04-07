/*!
 * This file is PART of plcdn_log_split
 * split nginx logs by domain and time, into multi-log-files
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 * 2.gcc enable c++11: -std=c++0x
 * 3.gcc include path add -I../inc/ or -I"${workspace_loc:/${ProjName}/inc}"
 * 4.gcc add -fPIC for shared libraries
 * 5.gcc add -lpopt -lpthread -lrt
 */

#if (defined __CYGWIN__ || (!defined _WIN32 && defined __GNUC__))

#include "test_options.h"	/*plcdn_ls_options**/
#include <time.h>		/*strftime*/
#include <stdio.h>
#include <string.h> 	/*strncpy*/
#include <sys/stat.h>	/*fstat*/
#include <sys/mman.h>	/*mmap*/
#include <string>		/*std::string*/
#include <unordered_map> 	/*std::unordered_map*/
#include <boost/filesystem.hpp> /*create_directories*/
#include <plcdn_la_ngx.h>	/*site_info*/
#include "plcdn_cpp_test.h"		/*test_plcdn_log_split_main*/


/////////////////////////////////////////////////////////////////////////////////////////////////////
/*nginx_log_analysis/parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*GLOBAL vars*/
/*plcdn_log_split/option.cpp*/
extern struct plcdn_ls_options plcdn_ls_opt;
/*map<domain, site_info>*/
static std::unordered_map<std::string, site_info> g_sitelist;

/////////////////////////////////////////////////////////////////////////////////////////////////////
static std::string parse_nginx_split_filename(char const * fmt, int site_id, struct tm& t)
{
	std::unordered_map<std::string, std::string> argmap;
	argmap["site_id"] = std::to_string(site_id);
	//TODO
	char buff[16];
	snprintf(buff, sizeof(buff), "%d%d%d", t.tm_year, t.tm_mon, t.tm_mday);
	argmap["day"] = buff;

	std::string outname;
	parse_fmt(fmt, outname, argmap);
	return outname;
}

/* parse $host and $request_time_msec from nginx log
 * log sample: 'flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] ...'
 * */
static int parse_nginx_log_request_time_msec_host_field(char const * p, char * domain, tm & t)
{
	auto c = strchr(p, ' ');
	if(!c) return -1;

	strncpy(domain, p, c - p);
	domain[c - p] = '\0';

	c = strchr(c, '[');
	if(!c || *(c - 1) !=  ' ')
		return -1;

	auto len = strchr(c, ']') - c - 1;
	char strtime[len + 1];
	strncpy(strtime, c + 1, len);
	strtime[len] = '\0';

	char const * result = strptime(c, "%d/%b/%Y:%H:%M:%S" , &t);
	if(!result)
		return -1;

	return 0;
}

static int split_nginx_log(FILE * f, char const * folder, char const * fmt)
{
	if(!f || !folder || folder[0] == '\0' || !fmt || fmt[0] == '\0')
		return -1;
	struct stat s;
	if(fstat(fileno(f), &s) < 0)
		return -1;
	/*FIXME: PAGE_SIZE?*/
	auto start_p = (char *)mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
	if(!start_p || start_p == MAP_FAILED)
		return -1;

	/*FIXME: see ulimit -n 'open files'*/
	std::unordered_map<std::string, FILE *> dmap; /*domain : log_file*/
	for(char const * p = start_p, * q = p; q != start_p + s.st_size; ++q){
		if(*q == '\n' && q - p > 0){
			auto data = p;
			auto len = q - p;
			p = q + 1;

			char domain[128];
			struct tm t;
			auto ret = parse_nginx_log_request_time_msec_host_field(data, domain, t);
			if(ret != 0){
				fprintf(stderr, "%s: parse_nginx_request_time_msec_host_field failed, skip\n", __FUNCTION__);
				continue;
			}

			int site_id = 0;
			find_site_id(g_sitelist, domain, site_id, NULL);
			auto && fname = parse_nginx_split_filename(fmt, site_id, t);
			auto && dname = (std::string(folder) + fname);
			auto status = boost::filesystem::create_directories(dname);
			if(!status){
				fprintf(stderr, "%s: create dir failed: '%s', skip\n", __FUNCTION__, dname.c_str());
				continue;
			}
			auto & file = dmap[fname];
			if(!file)
				file = fopen(dname.c_str(), "a");
			if(!file)
				return -1;

			int result = fwrite(data, sizeof(char), len, f);
			if(result < len || ferror(f)){
				fprintf(stderr, "%s: write error for domain: '%s'\n", __FUNCTION__, domain);
			}
		}
	}
	munmap(start_p, s.st_size);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
int test_plcdn_log_split_main(int argc, char ** argv)
{
	int result = plcdn_ls_parse_options(argc, argv);
	if(result != 0 || plcdn_ls_opt.show_help){
		plcdn_ls_opt.show_help? plcdn_ls_show_help(stdout) :
				plcdn_ls_show_usage(stdout);
		return 0;
	}
	if(plcdn_ls_opt.verbose)
		plcdn_ls_options_fprint(stdout, &plcdn_ls_opt);
	result = load_sitelist(plcdn_ls_opt.siteuidlist_file, g_sitelist);
	if(result != 0){
		fprintf(stderr, "%s: load_sitelist() failed\n", __FUNCTION__);
		return 1;
	}

	FILE * nginx_log_file = NULL/*, * srs_log_file = NULL*/;
	if(!plcdn_ls_opt.nginx_log_file){
		fprintf(stderr, "%s: nginx log file must specified\n", __FUNCTION__);
		return -1;
	}
	nginx_log_file = fopen(plcdn_ls_opt.nginx_log_file, "r");
	if(!nginx_log_file) {
		fprintf(stderr, "%s: fopen file '%s' failed\n", __FUNCTION__, plcdn_ls_opt.nginx_log_file);
		return 1;
	}

	if(plcdn_ls_opt.output_split_nginx_log){
		auto status = split_nginx_log(nginx_log_file,
				plcdn_ls_opt.output_split_nginx_log, plcdn_ls_opt.format_split_nginx_log);
		if(status != 0){
			return -1;
		}
	}
	return 0;
}

#endif /* defined __CYGWIN__ || (!defined _WIN32 && defined __GNUC__) */
////////////////////////////////////////////////////////////////////////////////////////////////////////////


