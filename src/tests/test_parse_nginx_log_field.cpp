/*!
 * @author hongjun.liao <docici@126.com>, @date 2017/03/07
 * @desc: test for parse one line of log(e.g. nginx's log) to log fields */

#include <stdio.h>

/* log_fied: a log_field is surrounded by delim and separated by split char: ' '
 * delim can be '\0', '[', ']', '"' etc.
 *
 * syntax:
 * (1)delim are always 'symmetrical', that is, for example, if begin with '[', then it must be end with ']', or a syntax error occurred
 * (2)' ' in fields are ignored, that is, for example, string '[07/Oct/2016:23:43:38 +0800]' is
 *    treated as 1 log_field, NOT 2('07/Oct/2016:23:43:38' and '+0800')
 *
 * sample nginx_log format:
 * '$host $remote_addr $request_time_msec $cache_status [$time_local] "$request_method \
 * $request_uri $server_protocol" $status $bytes_sent \
 * "$http_referer" "$remote_user" "$http_cookie" "$http_user_agent" \
 * $scheme $request_length $upstream_response_time'
 *
 * sample nginx_log:
 * flv.pptmao.com 183.240.128.180 14927 HIT [07/Oct/2016:23:43:38 +0800] \
 * "GET /2016-09-05/cbc1578a77edf84be8d70b97ba82457a.mp4 HTTP/1.1" 200 4350240 "http://www.pptmao.com/ppt/9000.html" \
 * "-" "-" "Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.0)" http 234 - CN4406 0E
 * */
struct log_field
{
	char delim;
	int index;
	char const * beg, * end;	/* [beg, end) */
};

int parse_to_log_fields(char const * str, log_field * fiels)
{
	fprintf(stderr, "%s: NOT implemented yet\n", __FUNCTION__);
	return -1;
}

int test_parse_to_log_fields_main(int argc, char ** argv)
{
	auto stream = stdin;
	if(argc > 1){
		stream = fopen(argv[1], "r");
		if(!stream){
			fprintf(stderr, "%s: fopen '%s' failed\n", __FUNCTION__, argv[1]);
			return -1;
		}
	}
	char buff[1024 * 64];
	while(fgets(buff, sizeof(buff), stream)){
		log_field fields[50];
		auto r = parse_to_log_fields(buff, fields);
		if(r != 0){
			fprintf(stderr, "%s: parse failed for '%s', skip\n", __FUNCTION__, buff);
			continue;
		}
		for(auto & item : fields){
		}
	}
	return 0;
}

