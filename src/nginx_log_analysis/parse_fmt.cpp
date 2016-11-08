/*!
 * This file is PART of nginx_log_analysis
 * parse formats in output,
 * e.g. parse "urlstat.${datetime}.${device_id}.${site_id}" to "urlstat.201608251005.2.1"
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include <stdio.h>
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 			/*std::string*/
#include <boost/regex.hpp> 		/*boost::regex_replace*/

int parse_fmt(char const * in, std::string& out,
		std::unordered_map<std::string, std::string> const& argmap)
{
	if(!in || in[0] == '\0') return -1;
	out = in;
	for(auto it : argmap){
		char buf[64];
		snprintf(buf, sizeof(buf), "\\$\\{%s\\}", it.first.c_str());
		boost::regex re(buf);
		out = boost::regex_replace(out, re, it.second);
	}
	return 0;
}


int test_nginx_log_parse_fmt_main(int argc, char ** argv)
{
	std::unordered_map<std::string, std::string> argmap = {
			{"datetime", "201608251005"},
			{"site_id", "1"},
			{"device_id", "2"},
			{"user_id", "3"},
	};
	auto filename = "urlstat.${datetime}.${device_id}.${site_id}";
	std::string outname;
	parse_fmt(filename, outname, argmap);
	fprintf(stdout, "%s: filename=%s, outname=%s\n", __FUNCTION__, filename, outname.c_str());
	return 0;
}

