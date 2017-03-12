/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/07

 * transform log format, currently only for nginx
 * @NOTE:
 * 1.facebook.folly used, see https://github.com/facebook/folly
 * clone:
 * (1)git clone http://git.oschina.net/SuzhenProjects/folly
 * (2)git remote add github https://github.com/facebook/folly.git
 * (3)git pull github master
 * build:
 * see https://github.com/facebook/folly
 * use in gcc:
 * -L ${folly}/.libs/ -lfollybase
 */
#include <stdio.h>
#include <string.h>				/* strcpy */
#include "nginx_log_analysis.h"	/* do_parse_nginx_log_item */
#include "test_options.h"	/* plcdn_la_options */
#ifdef USE_FACEBOOK_FOLLY
#include "folly/Format.h"	/* folly::svformat */
#endif /* USE_FACEBOOK_FOLLY */
/////////////////////////////////////////////////////////////////////////////////////////////////////
/* custom/user-defined nginx log format
 *
 * @note: currently plcdn nginx log format:
 * $host $remote_addr $request_time $upstream_cache_status [$time_local] "$request_method $request_uri $server_protocol" \
 * $status $bytes_sent "$http_referer" "$remote_user" "$http_cookie" "$http_user_agent" $scheme $request_length \
 * $upstream_response_time $body_bytes_sent "$http_x_forwarded_for" "$connection" "$server_addr"
 * */

/* user: yunduan, url: https://www.isurecloud.com/ */
#define CUSTOME_FORMAT_YUNDUAN_REGEX \
"$remote_addr  -  $remote_user [$time_local] \"$request_method $scheme://$host$request_uri $server_protocol\" $status \
$request_time $body_bytes_sent $bytes_sent \"$http_referer\" \
$http_user_agent \"$http_x_forwarded_for\"  \"$connection\" \"$HIT\" $server_addr"

#define CUSTOME_FORMAT_YUNDUAN_FOLLY \
"{remote_addr} - {remote_user} [{time_local}] \"{request_method} {scheme}://{host}{request_uri} {server_protocol}\" {status} \
{request_time} {body_bytes_sent} {bytes_sent} \"{http_referer}\" \
{http_user_agent} \"{http_x_forwarded_for}\" \"{connection}\" \"{HIT}\" {server_addr}"

/*plcdn_log_analysis/option.cpp*/
//extern struct plcdn_la_options plcdn_la_opt;

/*parse_fmt.cpp*/
extern int parse_fmt(char *in_out, std::unordered_map<std::string, std::string> const& argmap);

/* replace @param fmt with @param m, if NOT exist, use default value @param s
 * @note: 1.@param fmt is an in_and_out param */
static int do_nginx_transform_log(char** m, char * fmt, char const * s = "-");
/* override version */
static int do_nginx_transform_log(char** m, char const * fmt, std::string& out, char const * s = "-");

/* use boost::regex_replace */
static int do_nginx_transform_log(char** m, char * fmt, char const * s /*= "-"*/)
{
	if(!m || !fmt)
		return -1;
	std::unordered_map<std::string, std::string> argmap{
		{ "\\$host", (m[0]? m[0] : s) },  { "\\$remote_addr", (m[1]? m[1] : s) },  { "\\$request_time", (m[2]? m[2] : s) },
		{ "\\$upstream_cache_status", (m[3]? m[3] : s) },  { "\\$time_local", (m[4]? m[4] : s) },  { "\\$request_method", (m[5]? m[5] : s) },
		{ "\\$request_uri", (m[6]? m[6] : s) },  { "\\$server_protocol", (m[7]? m[7] : s) },  { "\\$status", (m[8]? m[8] : s) },
		{ "\\$bytes_sent", (m[9]? m[9] : s) },  { "\\$http_referer", (m[10]? m[10] : s) },  { "\\$remote_user", (m[11]? m[11] : s) },
		{ "\\$http_cookie", (m[12]? m[12] : s) },  { "\\$http_user_agent", (m[13]? m[13] : s) },  { "\\$scheme", (m[14]? m[14] : s) },
		{ "\\$request_length", (m[15]? m[15] : s) },  { "\\$upstream_response_time", (m[16]? m[16] : s) },  { "\\$body_bytes_sent", (m[17]? m[17] : s) },
		{ "\\$http_x_forwarded_for", (m[18]? m[18] : s) },  { "\\$connection", (m[19]? m[19] : s) },  { "\\$server_addr", (m[20]? m[20] : s) },
	};
	argmap["HIT"] = m[3]? (strncmp(m[3], "HIT", 3) == 0? "HIT" : "MISS") : s;

	char buff[1024 * 64];
	strncpy(buff, s, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s %s://%s%s %s", (m[5]? m[5] : s), (m[14]? m[14] : s),
			(m[0]? m[0] : s), (m[6]? m[6] : s), (m[7]? m[7] : s));
	argmap["request"] = buff;

	parse_fmt(fmt, argmap);
	return 0;
}

/* use facebook.folly::svformat */
static int do_nginx_transform_log(char** m, char const * fmt, std::string& out, char const * s /*= "-"*/)
{
	if(!m || !fmt)
		return -1;
	/* NOTE: there is NO '$' */
	std::unordered_map<std::string, std::string> argmap{
		{ "host", (m[0]? m[0] : s) },  { "remote_addr", (m[1]? m[1] : s) },  { "request_time", (m[2]? m[2] : s) },
		{ "upstream_cache_status", (m[3]? m[3] : s) },  { "time_local", (m[4]? m[4] : s) },  { "request_method", (m[5]? m[5] : s) },
		{ "request_uri", (m[6]? m[6] : s) },  { "server_protocol", (m[7]? m[7] : s) },  { "status", (m[8]? m[8] : s) },
		{ "bytes_sent", (m[9]? m[9] : s) },  { "http_referer", (m[10]? m[10] : s) },  { "remote_user", (m[11]? m[11] : s) },
		{ "http_cookie", (m[12]? m[12] : s) },  { "http_user_agent", (m[13]? m[13] : s) },  { "scheme", (m[14]? m[14] : s) },
		{ "request_length", (m[15]? m[15] : s) },  { "upstream_response_time", (m[16]? m[16] : s) },  { "body_bytes_sent", (m[17]? m[17] : s) },
		{ "http_x_forwarded_for", (m[18]? m[18] : s) },  { "connection", (m[19]? m[19] : s) },  { "server_addr", (m[20]? m[20] : s) },
	};
	argmap["HIT"] = m[3]? (strncmp(m[3], "HIT", 3) == 0? "HIT" : "MISS") : s;
#ifdef USE_FACEBOOK_FOLLY
	argmap["request"] = folly::svformat("{request_method} {scheme}://{host}{request_uri} {server_protocol}", argmap);
	out = folly::svformat(CUSTOME_FORMAT_YUNDUAN_FOLLY, argmap);
#endif /* USE_FACEBOOK_FOLLY */
	return 0;
}

int test_nginx_transform_log_main(int argc, char ** argv)
{
//	std::string stest = (argc > 1 ? argv[1] : "");
//	if(stest.empty() || stest == "help" || stest == "?" || stest == "-h" || stest == "--help"
//			|| stest == "-v" || stest == "--version"){
//		fprintf(stdout, "nginx log format transform tool, build at %s %s\n", __DATE__, __TIME__);
//		fprintf(stdout, "currently only for speed test. usage:\n");
//		fprintf(stdout, "cat bdrz.log | ./plcdn_logtrans > outfile\n");
//		return 0;
//	}

	FILE * in = stdin;
	FILE * out = stdout;
	int fmt = 2;
	if(!in || !out || fmt <= 0)
		return -1;
	char const * v[2] = { "[\"", "]\"" };
	char buff[1024 * 64];
	while(fgets(buff, sizeof(buff), in)){
		auto p = buff;
		char *items[60] = { 0 };
		int result = do_parse_nginx_log_item(items, p, v, {5}, '\n');
		if(result != 0){
//			if(plcdn_la_opt.verbose > 3)
//				fprintf(stderr, "%s: parse failed, skip:\n%s", __FUNCTION__, buff);
			continue;
		}
#ifndef USE_FACEBOOK_FOLLY
		/*use boost::regex_replace */
		fprintf(stdout, "%s: WARNING, facebook.folly disabled, use boost.regex instead(poor efficiency)\n", __FUNCTION__);
//		char const * fmtstr = "05 07 \"08.1://08.2";
		char outbuff[1024 * 64];
		strcpy(outbuff, (fmt == 2? CUSTOME_FORMAT_YUNDUAN_REGEX : ""));
		result = do_nginx_transform_log(items, outbuff);
#else
		/*use folly::svformat */
		std::string outstr;
		result = do_nginx_transform_log(items, CUSTOME_FORMAT_YUNDUAN_FOLLY, outstr);
		auto outbuff = outstr.c_str();
#endif /* USE_FACEBOOK_FOLLY */
		if(result != 0){
//			if(plcdn_la_opt.verbose > 3)
//				fprintf(stderr, "%s: transform failed, skip:\n%s", __FUNCTION__, buff);
			continue;
		}
		if(fprintf(out, "%s\n", outbuff) == EOF){
//			if(plcdn_la_opt.verbose > 3)
//				fprintf(stderr, "%s: fputs failed, skip:\n%s", __FUNCTION__, outbuff);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "bd_test.h"
#include <unistd.h>	/* sysconf */
#include <sys/uio.h>	/* iovec */
#include <limits.h>		/* IOV_MAX */
#include <stdlib.h>

typedef std::pair<char const *, char const *> field_t;
#define spair_size(p)  (p.second - p.first)

struct logtrans_fmt
{
	char const * beg, * end;
	int i;	/* -1: use [beg, end); -3: use '%', >=0: use i*/
};

static char const * array_rchr(char const * buf, int sz, char ch = '\n');
static int myatoi(char const * beg, char const * end);

/*
 * @param fmtf, @param m: format
 * @param arg, @param n: args
 * @param rn: field count for 1 line
 * @return:
 * -2, malloc failed
 * -3, index out of range
 * 0, success
 * */
static int pl_logtrans_printf(logtrans_fmt * fmtf, size_t m, field_t const * arg, size_t n,
		int rn, FILE * f, char const * def = "-");
/* compile format string, sample "%1 - [%2] \"%3\" %% %4" */
static int pl_logtrans_compile_format(char const * fmt, int rn, logtrans_fmt * fmtf, size_t & m);

static char const * array_rchr(char const * buf, int sz, char ch /*= '\n'*/)
{
	for(auto const * p = buf + sz - 1; p != buf - 1; --p) {
		if(*p == ch)
			return p;
	}
	return NULL;
}

static inline int myatoi(char const * begin, char const * end)
{
	char buf[64] = "";
	strncpy(buf, begin, end - begin);
	return atoi(buf);
}

/* sample "%1 - [%2] \"%3\" %% %4a %1" */
static int pl_logtrans_compile_format(char const * fmt, int rn, logtrans_fmt * fmtf, size_t & m)
{
	auto end = fmt + strlen(fmt);
	m = 0;
	for(auto p = fmt, q = p; ;++q){ /* compile format */
		if(q == end){
			if(p != q) /* end of format string */
				fmtf[m++] = logtrans_fmt{p, q, -1}; /* FIXME: checks for fmtf[m] out of range */
			break;
		}
		if(*q == '%'){
			if(q + 1 == end)
				return -1;	/* syntax error */
			auto s = q + 1;
			if(isdigit((int)*s)){
				do{ ++s; } while(isdigit((int)*s));
				auto i = myatoi(q + 1, s);
				if(i + 1 > rn)
					return -3; /* out of range */

				if(p != q)
					fmtf[m++] = logtrans_fmt{p, q, -1};
				fmtf[m++] = logtrans_fmt{0, 0, i};
				p = s;
				q = s - 1;
				continue;	/* in next loop p == q */
			}
			if(*s == '%'){	/* %% as % */
				if(p != q)
					fmtf[m++] = logtrans_fmt{p, q, -1};
				fmtf[m++] = logtrans_fmt{0, 0, -3};
				q = s;
				p = s + 1;
				continue; /* in next loop p == q */
			}
			return -1;	/* syntax error: '%' or '%a' NOT allowed */
		}
	}
//	for(size_t i = 0; i < m; ++i){
//		if(fmtf[i].i >= 0){
//			fprintf(stderr, "%zu, use index %d\n", i, fmtf[i].i);
//		}
//		else if(fmtf[i].i == -1){
//			fprintf(stderr, "%zu, use [beg, end):'", i);
//			for(auto it = fmtf[i].beg; it != fmtf[i].end; ++it)
//				switch(*it){
//				case '\n':
//					fprintf(stderr, "\\n");
//					break;
//				default:
//					fprintf(stderr, "%c", *it);
//				}
//			fprintf(stderr, "'\n");
//		}
//		else if(fmtf[i].i == -3){
//			fprintf(stderr, "%zu, use '%%'\n", i);
//		}
//		else
//			fprintf(stderr, "%zu, error %d\n", i, fmtf[i].i);
//	}
//	exit(0);
	return 0;
}

static int pl_logtrans_printf(logtrans_fmt * fmtf, size_t m, field_t const * arg, size_t n,
		int rn, FILE * f, char const * def /*= "-"*/)
{
	auto DEF_LEN = strlen(def);
	size_t VEC = rn * 2 * (n / rn);
	auto vec = (iovec * )malloc(sizeof(iovec) * VEC);
	if(!vec){
		fprintf(stderr, "%s: malloc for %zu iovec failed, exit\n", __FUNCTION__, VEC);
		return -2;
	}

	size_t k = 0;
	char ch[] = "%";
	for(size_t i = 0, row = n / rn; i < row; ++i){ 	 /*  for each line */
		for(size_t j = 0; j < m; ++j){
			char const * base = NULL;
			size_t len = 0;
			if(fmtf[j].i >= 0){
				auto ii = fmtf[j].i +  i * rn;
				if(arg[ii].first){
					base = arg[ii].first;
					len = arg[ii].second - arg[ii].first;
				}
				else{
					base = def;
					len = DEF_LEN;
				}
			}
			else if(fmtf[j].i == -1){
				base = fmtf[j].beg;
				len = fmtf[j].end - fmtf[j].beg;
			}
			else if(fmtf[j].i == -3){
				base = ch;
				len = 1;
			}
			if(base && len > 0){
				vec[k].iov_base = const_cast<char *>(base);
				vec[k].iov_len = len;
				++k;
			}
		}
	}
	auto iov_max = sysconf(_SC_IOV_MAX);
	if(iov_max == -1)
		iov_max = IOV_MAX;
//	fprintf(stdout, "%s: fmtf_m=%zu, arg_n=%zu, rn=%d, def='%s', iov_max=%ld, k=%zu\n", __FUNCTION__,
//			m, n, rn, def, iov_max, k);

	/* write to file */
	size_t w = 0;
	for(size_t i = 0; w != k; ++i){
		auto M = (k - w > (size_t)iov_max? iov_max : k - w);
		auto r = writev(fileno(f), vec + i * iov_max, M);
		if(r < 0){
			fprintf(stderr, "%s: writev failed\n", __FUNCTION__);
			free(vec);
			return -1;
		};
		w += M;
	}
	free(vec);
	return 0;
}

int pl_logtrans_trans_file(char const * fmt, int rn, FILE * in, FILE * out)
{
	size_t arn = rn * 8;
	logtrans_fmt fmtf[arn];	/* FIXME: rn * 8 at max */
	auto result = pl_logtrans_compile_format(fmt, rn, fmtf, arn);
	if(result != 0){
		fprintf(stderr, "%s: compile format '%s' failed, please check syntax. exit\n", __FUNCTION__, fmt);
		return -1;
	}

	size_t BUF = 1024 * 1024 * 128, ROW = 1024 * 64;
	size_t FIELD = 1024 * 1024 * 60;	/* avg 120bytes per line */
	char const * v[2] = { "[\"", "]\"" };

	auto fc = FIELD;

	auto buf = (char *)malloc(sizeof(char) * (BUF + ROW));
	auto fields = (field_t *)malloc(sizeof(field_t) * FIELD);
	if(!buf || !fields){
		fprintf(stderr, "%s: malloc for %.0f MB failed, exit\n", __FUNCTION__,
				(BUF + ROW + FIELD) / (1024.0 * 1014.0));
		return -1;
	}
	char * p = buf;
	size_t count = 0;
	for(;; ++count){
		auto sz = fread(p, sizeof(char), BUF, in);
		if(ferror(in)){
			fprintf(stderr, "%s: fread failed, exit\n", __FUNCTION__);
			return -1;
		}
		if(sz == 0)
			break;
		auto end = array_rchr(buf, sz + p - buf);
		if(!end)
			continue;	/* no '\n' and no error, a huge-long line? */
		++end;	/* skip '\n' */
		auto result = do_parse_nginx_log_item(fields, fc, rn, buf, end - buf, v, 5);
		if(result != 0 || (fc % rn != 0)){
			fprintf(stderr, "%s: parse failed, skip, ret=%d\n", __FUNCTION__, result);
			return -1;
		}
		result = pl_logtrans_printf(fmtf, arn, fields, fc, rn, out);
		if(result != 0){
			fprintf(stderr, "%s: pl_logtrans_printf failed, exit, ret=%d\n", __FUNCTION__, result);
			return -1;
		}
		fc = FIELD;
		auto len = p + sz - end;
		memmove(buf, end, len);
		p = buf + len;
	}
	if(p != buf){
		fprintf(stderr, "%s: parse failed, exit\n", __FUNCTION__);
	}
	fprintf(stderr, "%s: processed, loop=%zu\n", __FUNCTION__, count);
	free(buf);
	free(fields);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/* a faster version */
int test_nginx_transform_log_main_2(int argc, char ** argv)
{
	size_t RN = 21;	/* field count for original file line */
	/* "%1 - %11 [%4] %5 \"%14://%0%6 %7\" %8 %2 %17 %9 \"%10\" %13 \"%18\" \"%19\" \"HIT\" %20\n" */
	return pl_logtrans_trans_file("%1 - %11 [%4] %5 \"%14://%0%6 %7\" %8 %2 %17 %9 \"%10\" %13 \"%18\" \"%19\" \"HIT\" %20\n", RN, stdin, stdout);
}

