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
/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

char const * g_ngx_v[2] = { "[\"", "]\"" };

static std::map<std::string, int *> nginx_log_items {
		{ "host", &plcdn_la_opt.ngx_logfmt.host },
		{ "remote_addr", &plcdn_la_opt.ngx_logfmt.remote_addr },
		{ "request_time_msec", &plcdn_la_opt.ngx_logfmt.request_time_msec },
		{ "upstream_cache_status", &plcdn_la_opt.ngx_logfmt.upstream_cache_status },
		{ "time_local", &plcdn_la_opt.ngx_logfmt.time_local },
		{ "request_method", &plcdn_la_opt.ngx_logfmt.request_method },
		{ "request_uri", &plcdn_la_opt.ngx_logfmt.request_uri },
		{ "server_protocol", &plcdn_la_opt.ngx_logfmt.server_protocol },
		{ "status", &plcdn_la_opt.ngx_logfmt.status },
		{ "bytes_sent", &plcdn_la_opt.ngx_logfmt.bytes_sent },
		{ "http_referer", &plcdn_la_opt.ngx_logfmt.http_referer },
		{ "remote_user", &plcdn_la_opt.ngx_logfmt.remote_user },
		{ "http_user_agent", &plcdn_la_opt.ngx_logfmt.http_user_agent },
		{ "scheme", &plcdn_la_opt.ngx_logfmt.scheme },
		{ "request_length", &plcdn_la_opt.ngx_logfmt.request_length },
		{ "upstream_response_time", &plcdn_la_opt.ngx_logfmt.upstream_response_time },
		{ "start_response_time_msec", &plcdn_la_opt.ngx_logfmt.start_response_time_msec },
		{ "body_bytes_sent", &plcdn_la_opt.ngx_logfmt.body_bytes_sent },
		{ "http_x_forwarded_for", &plcdn_la_opt.ngx_logfmt.http_x_forwarded_for },
		{ "connection", &plcdn_la_opt.ngx_logfmt.connection },
		{ "server_addr", &plcdn_la_opt.ngx_logfmt.server_addr },

		{ "site_id", &plcdn_la_opt.ngx_logfmt.site_id },
		{ "upstream_addr", &plcdn_la_opt.ngx_logfmt.upstream_addr },
		{ "log_cache_uri", &plcdn_la_opt.ngx_logfmt.log_cache_uri },
};

/*
 * parse nginx_log_format, sample:
 *
 * '$host $remote_addr $request_time_msec \
 * $upstream_cache_status [$time_local] \"$request_method $request_uri $server_protocol" \
 * $status $bytes_sent "$http_referer" "$remote_user" - "$http_user_agent" $scheme $request_length $upstream_response_time \
 * $start_response_time_msec $body_bytes_sent "$http_x_forwarded_for" "$connection" "$server_addr"'
 *
 * @return 0 on success
 */
void ngx_parse_nginx_log_format(char const * str, ngx_log_format & fmt)
{
	memset(fmt.sub_items, 0, sizeof(fmt.sub_items));
	fmt.n_sub = 0;

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
					fprintf(stdout, "%s: found item: %34s, i=%2d\n", __FUNCTION__, s.c_str(), i);
				*nginx_log_items[s] = i;
			}
			p = e - 1;
		}
	}

	if(fmt.n_sub > 0 && plcdn_la_opt.verbose > 7){
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

	ngx_log_format fmt;
	ngx_parse_nginx_log_format(str, fmt);

	return 0;
}
