/*!
 * This file is PART of nginx_log_analysis
 * parse option
 * e.g. parse "-u tq_20160825/url_popular/,split=1,format=urlstat.${datetime}.${device_id}.${site_id}", and get split, format
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include <stdio.h>
#include <string.h>
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 			/*std::string*/

int parse_option(char * in, std::unordered_map<std::string, char *> & out)
{
	if(!in || in[0] == '\0') return -1;
	auto token = strtok(in, ",");
	while(token) {
		auto val = strchr(token, '=');
		if(!val) return -1;
		*val = '\0';
		++val;
		out[token] = val;

		token = strtok(NULL, ",");
	}
	return 0;
}


int test_nginx_log_parse_option_main(int argc, char ** argv)
{
	std::unordered_map<std::string, char *> out;
	char p[] = "split=1,format=urlstat.${datetime}.${device_id}.${site_id}";
	fprintf(stdout, "%s: options=%s\n", __FUNCTION__, p);
	if(parse_option(p, out) != 0)
		return -1;
	for(auto it : out){
		fprintf(stdout, "\t%s=%s\n", it.first.c_str(), it.second);
	}
	return 0;
}

