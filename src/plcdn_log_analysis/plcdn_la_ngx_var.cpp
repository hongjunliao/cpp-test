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

int parse_fmt(char *in_out, std::unordered_map<std::string, std::string> const& argmap)
{
	if(!in_out || in_out[0] == '\0')
		return -1;
	std::string out = in_out;
	for(auto it : argmap){
		boost::regex re(it.first);
		out = boost::regex_replace(out, re, it.second);
	}
	strcpy(in_out, out.c_str());
	return 0;
}

/* parse format like -e 'hello,fmt=file.${interval}', the results should be:
 * -e -> hello
 * fmt -> file.${interval}
 *
 * rules:
 * (1)if has multiple args, ',' as the split char
 * (2)'=' for separate key and value
 * (3)without blanks
 *
 * @NOTE: this version only support 1 key
 *
 * @param buf:    buffer to parse
 * @param arg:    output, value, e.g. 'hello'
 * @param key:    input, key, e.g. 'fmt'
 * @param value:  output, value, e.g.  'file.${interval}'
 */
void parse_args_split(char * buf, char const *& arg, char const * key, char const *& value)
{
	arg = buf;

	auto c1 = strchr(buf, ',');
	if(c1){
		auto c2 = strchr(c1 + 1, '=');
		if(c2){
			int len = strlen(key);
			if((c2 - (c1 + 1) == len) && strncmp(c1 + 1, key, len) == 0){
				*c1 = '\0';

				value = c2 + 1;

				return;
			}
		}
	}
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

