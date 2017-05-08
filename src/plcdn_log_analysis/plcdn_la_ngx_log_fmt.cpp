/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/30
 *
 * parse nginx log format
 */
#include "plcdn_la_ngx.h"	/* ngx_log_format */
#include <string.h>	        /* strlen */
#include "plcdn_la_option.h"	/*plcdn_la_options*/
/* plcdn_la_option.cpp */
extern struct plcdn_la_options plcdn_la_opt;

char const * g_ngx_v[2] = { "[\"", "]\"" };

/*
 * parse nginx_log_format, sample:
 *
 * '$host $remote_addr $request_time_msec \
 * $upstream_cache_status [$time_local] \"$request_method $request_uri $server_protocol" \
 * $status $bytes_sent "$http_referer" "$remote_user" - "$http_user_agent" $scheme $request_length $upstream_response_time \
 * $start_response_time_msec $body_bytes_sent "$http_x_forwarded_for" "$connection" "$server_addr"'
 */
void ngx_parse_nginx_log_format(char const * str, ngx_log_format & fmt)
{
	memset(&fmt, -1, sizeof(fmt));
	memset(fmt.sub_items, 0, sizeof(fmt.sub_items));
	fmt.n_sub = 0;

	std::map<std::string, int *> nginx_log_items {
			{ "host", &fmt.host },
			{ "remote_addr", &fmt.remote_addr },
			{ "request_time_msec", &fmt.request_time_msec },
			{ "upstream_cache_status", &fmt.upstream_cache_status },
			{ "time_local", &fmt.time_local },
			{ "request_method", &fmt.request_method },
			{ "request_uri", &fmt.request_uri },
			{ "server_protocol", &fmt.server_protocol },
			{ "status", &fmt.status },
			{ "bytes_sent", &fmt.bytes_sent },
			{ "http_referer", &fmt.http_referer },
			{ "remote_user", &fmt.remote_user },
			{ "http_user_agent", &fmt.http_user_agent },
			{ "scheme", &fmt.scheme },
			{ "request_length", &fmt.request_length },
			{ "upstream_response_time", &fmt.upstream_response_time },
			{ "start_response_time_msec", &fmt.start_response_time_msec },
			{ "body_bytes_sent", &fmt.body_bytes_sent },
			{ "http_x_forwarded_for", &fmt.http_x_forwarded_for },
			{ "connection", &fmt.connection },
			{ "server_addr", &fmt.server_addr },

			{ "site_id", &fmt.site_id },
			{ "upstream_addr", &fmt.upstream_addr },
			{ "log_cache_uri", &fmt.log_cache_uri },

			{ "header_start_msec", &fmt.header_start_msec },
			{ "upstream_status", &fmt.upstream_status },
			{ "upstream_total_received", &fmt.upstream_total_received },
			{ "upstream_response_msec", &fmt.upstream_response_msec },
			{ "log_server_addr", &fmt.log_server_addr },
	};

	auto v = g_ngx_v;

	if(plcdn_la_opt.verbose > 7)
		fprintf(stdout, "%s: format='%s'\n", __FUNCTION__, str);

	auto end = str + strlen(str);

	int i = -1;
	char const * ch = 0;
	int n_ch = 0;

	for(auto p = str; p != end; ++p){
		auto c = strchr(v[0], *p);
		if(c && !ch){ /* found border_begin */
			ch = c;
			continue;
		}
		if(ch && *p == v[1][ch - v[0]]){ /* found border_end */
			if(n_ch > 1)	/* i > -1 */
				fmt.sub_items[fmt.n_sub++] = i - n_ch + 1;

			ch = 0;
			n_ch = 0;

			continue;
		}
		if(*p == '$'){
			if(p + 1 != end && *(p + 1) == '$')
				continue;
			if(ch)
				++n_ch;

			auto e = p + 1;
			while((*e >= '0' && *e <= '9') || (*e >= 'a' && *e <= 'z') || (*e >= 'A' && *e <= 'Z') || *e == '_')
				++e;

			++i;

			std::string s{ p + 1, e };
			if(nginx_log_items.count(s) != 0){
				if(plcdn_la_opt.verbose > 7)
					fprintf(stdout, "%s: found field: %34s, i=%2d\n", __FUNCTION__, s.c_str(), i);
				*nginx_log_items[s] = i;
			}
			p = e - 1;
		}
	}

	if(fmt.n_sub > 0 && plcdn_la_opt.verbose > 7){
		fprintf(stdout, "%s: fields NOT found: [", __FUNCTION__);
		for(auto & it : nginx_log_items){
			if(*it.second >= 0)
				continue;
			auto color = 31; /* red */
			fprintf(stdout, "\e[%dm%s, \e[0m", color, it.first.c_str());
		}
		fprintf(stdout, "]\n");

		fprintf(stdout, "%s: n_sub=%d, sub_items: [", __FUNCTION__, fmt.n_sub);
		for(int i = 0; i < fmt.n_sub; ++i)
			fprintf(stdout, "%d,", fmt.sub_items[i]);
		fprintf(stdout, "]\n");
	}
}

int test_ngx_parse_nginx_log_format_main(int argc, char ** argv)
{
	auto str = "$host $remote_addr $request_time_msec $upstream_cache_status [$time_local] \"$request_method $request_uri $server_protocol "
		      "$status $bytes_sent \"$http_referer\" \"$remote_user\" - \"$http_user_agent\" $scheme $request_length $upstream_response_time "
		      "$start_response_time_msec $body_bytes_sent \"$http_x_forwarded_for\" \"$connection\" \"$server_addr\"";

	fprintf(stdout, "%s: nginx_log_format:\n'%s'\n", __FUNCTION__, str);

	ngx_log_format fmt = plcdn_la_opt.ngx_logfmt;
	ngx_parse_nginx_log_format(str, fmt);

	plcdn_la_opt.ngx_logfmt = fmt;
	return 0;
}
