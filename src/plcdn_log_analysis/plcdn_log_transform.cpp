/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/07

 * transform log format, currently only for nginx
 * @NOTE:
 * 1.lib facebook/folly used, see https://github.com/facebook/folly, following
 *
 * (1)git clone git clone  http://git.oschina.net/SuzhenProjects/folly
 * (2)git remote add git remote add github https://github.com/facebook/folly.git
 * (3)git pull gibhub master
 */
#include <stdio.h>
#include <string.h>				/* strcpy */
#include "nginx_log_analysis.h"	/* do_parse_nginx_log_item */
#include "test_options.h"	/* plcdn_la_options */
//#include "folly/Format.h"	/* folly::svformat; */
/////////////////////////////////////////////////////////////////////////////////////////////////////
/* custom/user-defined nginx log format
 *
 * @note: currently plcdn nginx log format:
 *   $host $remote_addr $request_time $upstream_cache_status [$time_local] "$request_method $request_uri $server_protocol"
 *   $status $bytes_sent "$http_referer" "$remote_user" "$http_cookie" "$http_user_agent" $scheme $request_length
 *   $upstream_response_time $body_bytes_sent "$http_x_forwarded_for" "$connection" "$server_addr"
 * */

/* user: yunduan, url: https://www.isurecloud.com/ */
#define CUSTOME_FORMAT_YUNDUAN \
"$remote_addr  -  $remote_user [$time_local] \"$request_method $scheme://$host$request_uri $server_protocol\" $status \
$request_time $body_bytes_sent $bytes_sent \"$http_referer\" \
$http_user_agent \"$http_x_forwarded_for\"  \"$connection\" \"$HIT\" $server_addr"

/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

/*parse_fmt.cpp*/
extern int parse_fmt(char *in_out,
		std::unordered_map<std::string, std::string> const& argmap);

/* replace @param fmt with @param m, if NOT exist, use default value @param s
 * @note:
 * 1.@param fmt is an in_and_out param */
static int do_nginx_transform_log(char** m, char * fmt, char const * s = "-");

static int do_nginx_transform_log_boost_regex_replace(char** m, char * fmt, char const * s /*= "-"*/)
{
	if(!m || !fmt)
		return -1;
	std::unordered_map<std::string, std::string> argmap{
		{ "\\$host", (m[0]? m[0] : s) },  { "\\$remote_addr", (m[1]? m[1] : s) },  { "\\$request_time", (m[2]? m[2] : s) },
		{ "\\$upstream_cache_status", (m[3]? m[3] : s) }, /* { "\\$time_local", (m[0]? m[0] : s) },*/  { "\\$request_method", (m[0]? m[0] : s) },
		{ "\\$request_uri", (m[6]? m[6] : s) },  { "\\$server_protocol", (m[0]? m[0] : s) },  { "\\$status", (m[0]? m[0] : s) },
		{ "\\$bytes_sent", (m[9]? m[9] : s) },  { "\\$http_referer", (m[0]? m[0] : s) },  { "\\$remote_user", (m[0]? m[0] : s) },
		{ "\\$http_cookie", (m[12]? m[12] : s) },  { "\\$http_user_agent", (m[0]? m[0] : s) },  { "\\$scheme", (m[0]? m[0] : s) },
		{ "\\$request_length", (m[15]? m[15] : s) },  { "\\$upstream_response_time", (m[0]? m[0] : s) },  { "\\$body_bytes_sent", (m[0]? m[0] : s) },
		{ "\\$http_x_forwarded_for", (m[18]? m[18] : s) },  { "\\$connection", (m[0]? m[0] : s) },  { "\\$server_addr", (m[0]? m[0] : s) },
	};

	/* FIXME: ugly */
	char time_local[sizeof("07/Oct/2016:22:32:16 +0800") + 1];
	strcpy(time_local, s);
	if(m[4] && m[5]){
		*(m[5] + 5) = '\0'; 	/* delete ']'*/
		sprintf(time_local, "%s %s", m[4] + 1, m[5]);
	}
	argmap["\\$time_local"] = time_local;
	parse_fmt(fmt, argmap);
	return 0;
}

static int do_nginx_transform_log_facebook_folly(char** m, char * fmt, char const * s /*= "-"*/)
{
	if(!m || !fmt)
		return -1;
	std::unordered_map<std::string, std::string> argmap{
		{ "$host", (m[0]? m[0] : s) },  { "$remote_addr", (m[1]? m[1] : s) },  { "$request_time", (m[2]? m[2] : s) },
		{ "$upstream_cache_status", (m[3]? m[3] : s) },  { "$time_local", (m[0]? m[0] : s) },  { "$request_method", (m[0]? m[0] : s) },
		{ "$request_uri", (m[6]? m[6] : s) },  { "$server_protocol", (m[0]? m[0] : s) },  { "$status", (m[0]? m[0] : s) },
		{ "$bytes_sent", (m[9]? m[9] : s) },  { "$http_referer", (m[0]? m[0] : s) },  { "$remote_user", (m[0]? m[0] : s) },
		{ "$http_cookie", (m[12]? m[12] : s) },  { "$http_user_agent", (m[0]? m[0] : s) },  { "$scheme", (m[0]? m[0] : s) },
		{ "$request_length", (m[15]? m[15] : s) },  { "$upstream_response_time", (m[0]? m[0] : s) },  { "$body_bytes_sent", (m[0]? m[0] : s) },
		{ "$http_x_forwarded_for", (m[18]? m[18] : s) },  { "$connection", (m[0]? m[0] : s) },  { "$server_addr", (m[0]? m[0] : s) },
	};
	/* FIXME: ugly */
	char time_local[sizeof("07/Oct/2016:22:32:16 +0800") + 1];
	strcpy(time_local, s);
	if(m[4] && m[5]){
		*(m[5] + 5) = '\0'; 	/* delete ']'*/
		sprintf(time_local, "%s %s", m[4] + 1, m[5]);
	}

	return 0;
}


static int do_nginx_transform_log(char** m, char * fmt, char const * s /*= "-"*/)
{
	return do_nginx_transform_log_boost_regex_replace(m, fmt, s);
}

int nginx_transform_log(FILE * in, FILE * out, int fmt)
{
	if(!in || !out || fmt <= 0)
		return -1;
	char buff[1024 * 64];
	while(fgets(buff, sizeof(buff), in)){
		auto p = buff;
		char *items[60] = { 0 };
		int result = do_parse_nginx_log_item(items, p, '\n');
		if(result != 0){
			if(plcdn_la_opt.verbose > 3)
				fprintf(stderr, "%s: parse failed, skip:\n%s", __FUNCTION__, buff);
			continue;
		}
//		char fmtstr = "05 07 \"08.1://08.2";
		char outbuff[1024 * 64];
		strcpy(outbuff, (fmt == 2? CUSTOME_FORMAT_YUNDUAN : ""));

		result = do_nginx_transform_log(items, outbuff);
		if(result != 0){
			if(plcdn_la_opt.verbose > 3)
				fprintf(stderr, "%s: transform failed, skip:\n%s", __FUNCTION__, buff);
			continue;
		}
		if(fprintf(out, "%s\n", outbuff) == EOF){
			if(plcdn_la_opt.verbose > 3)
				fprintf(stderr, "%s: fputs failed, skip:\n%s", __FUNCTION__, outbuff);
		}
	}
	return 0;
}

