/*!
 * This file is PART of plcdn_log_analysis
 * nginx local_url_key
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/15
 */
#include "nginx_log_analysis.h"	/* nginx_domain_stat, ...*/
#include "string.h"				/* strlen */
#include <unordered_map> 		/*std::unordered_map*/
#include <string>				/*std::string*/
#include <boost/filesystem.hpp> /*create_directories*/

/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

/*parse_fmt.cpp*/
extern int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap);

static std::string parse_nginx_local_url_key_filename(char const * fmt, int site_id, int user_id);

/* read url_key from file @param f and insert to @param m */
static int nginx_insert_url_key_map(std::unordered_map<std::string, std::string> & m, FILE * f);
/* @param n: failed lines */
static int do_fwrite_nginx_local_url_key(std::unordered_map<std::string, std::string> const & m, FILE * f, size_t& n);

static std::string parse_nginx_local_url_key_filename(char const * fmt, int site_id, int user_id)
{
	std::unordered_map<std::string, std::string> argmap;
	argmap["site_id"] = std::to_string(site_id);
	argmap["user_id"] = std::to_string(user_id);

	std::string outname;
	parse_fmt(fmt, outname, argmap);
	return outname;
}

static int nginx_insert_url_key_map(std::unordered_map<std::string, std::string> & m, FILE * f)
{
	char a[128], b[1024 * 64];
	a[0] = b[0] = '\0';
	for(auto r = 0; (r = fscanf(f, "%s%s", a, b)) != EOF; ){
		if(r != 2)
			continue;
		m[a] = b;
	}
	return 0;
}

static int do_fwrite_nginx_local_url_key(std::unordered_map<std::string, std::string> const & m, FILE * f, size_t& n)
{
//	fprintf(stdout, "%s: \n", __FUNCTION__);
	/* TODO: use iovec/writev? */
    for(auto const & item : m){
		auto sz = fprintf(f, "%s %s\n", item.first.c_str(), item.second.c_str());
		if(sz <= 0)
			++n;
    }
    return 0;
}

int fwrite_nginx_local_url_key(std::unordered_map<std::string, nginx_domain_stat> const& stats, char const * fmt)
{
	if(!fmt || !fmt[0])
		return -1;

	/* get folder name */
	std::string folder, fname;
	auto fmtlen = strlen(fmt);
	for(auto p = fmt + fmtlen - 1 ; p != fmt - 1; --p){
		if(*p == '/' || *p == '\\'){
			folder.assign(fmt, p + 1);
			if(p == fmt + fmtlen - 1)
				fname = "${site_id}";
			else
				fname.assign(p + 1, fmt + fmtlen);
			break;
		}
	}
//	fprintf(stdout, "%s: folder='%s',fname='%s'\n", __FUNCTION__, folder.c_str(), fname.c_str());
	/* create folder */
	boost::system::error_code ec;
	if(!boost::filesystem::is_directory(folder, ec)){
		auto ret =  boost::filesystem::create_directories(folder, ec);
		/* FIXME: dirs are created in folder, but ret == 0, I don't know y, use is_directory instead */
		if(!boost::filesystem::is_directory(folder, ec)){
			if(plcdn_la_opt.verbose){
				fprintf(stderr, "%s: create_directories '%s' failed in '%s'\n", __FUNCTION__,
						folder.c_str(), fmt);
			}
			return -1;
		}
	}

	/* append first */
	size_t n = 0;
	std::map<std::string, FILE *> filemap; /*for output filenames*/
	for(auto const& dstat : stats){
		auto sid = folder + parse_nginx_local_url_key_filename(fname.c_str(), dstat.second._site_id, dstat.second._user_id);
		auto & f = filemap[sid];
		if(!f){
			f = fopen(sid.c_str(), "a");
			if(!f)
				continue; /* disk error? */
		}
		do_fwrite_nginx_local_url_key(dstat.second._url_key, f, n);
	}
	/* remove duplicates */
	for (auto & it : filemap) {
		auto f = it.second;
		if (!f)
			continue;
		f = freopen(NULL, "r", f);
		if(!f)
			continue;
		std::unordered_map<std::string, std::string> ukmap;	/* urlkey: url */
		nginx_insert_url_key_map(ukmap, f);
		f = freopen(NULL, "w", f);
		if(!f)
			continue;
		do_fwrite_nginx_local_url_key(ukmap, f, n);
		fclose(it.second);
	}
	if(n != 0)
		fprintf(stderr, "%s: WARNING, skip %zu lines\n", __FUNCTION__, n);
	return 0;
}
