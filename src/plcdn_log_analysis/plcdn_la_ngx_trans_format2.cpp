/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/07

 * transform log format, currently only for nginx
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include <unistd.h>	/* sysconf */
#include <sys/uio.h>	/* iovec */
#include <limits.h>		/* IOV_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* strncpy */
#include <utility>		/* std::pair */
#include <ctype.h>		/* isdigit */
#include <plcdn_la_ngx.h>	/* do_parse_nginx_log_item */

typedef std::pair<char const *, char const *> field_t;
#define spair_size(p)  (p.second - p.first)

struct logtrans_fmt
{
	char const * beg, * end;
	int i;	/* -1: use [beg, end); -3: use '%', >=0: use i*/
	void * arg;
	int (*fn)(char const * beg, char const * end, void * arg, char const *& vec, size_t & len);
};

/* user-defiend functions for logtrans_fmt.fn */
struct logtrans_match_arg
{
	char const * left, *lend;
	char const * right, * rend;

};

/* for syntax: '%N?left:right%' */
static int logtrans_match(char const * beg, char const * end, void * arg, char const *& vec, size_t & len);

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
static void pl_logtrans_free_format(logtrans_fmt * fmtf, size_t const & m);

static int logtrans_match(char const * beg, char const * end, void * arg, char const *& vec, size_t & len)
{
	auto argv = (logtrans_match_arg *)arg;
	if(!argv)
		return -1;
	auto flen = end - beg;
	auto llen = argv->lend - argv->left;
	auto rlen = argv->rend - argv->right;
	if(llen <= 0 || flen < llen || rlen <= 0)
		return -1;
	if(memcmp(beg, argv->left, llen) == 0){
		vec = argv->left;
		len = llen;
	}
	else{
		vec = argv->right;
		len = rlen;
	}
	return 0;
}

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

/* sample "%1 - %11 [%4] %5 \"%14://%0%6 %7\" %8 %2 %17 %9 \"%10\" %13 \"%18\" \"%19\" \"%3?HIT:MISS\" %20\n" */
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
				if(*s == '?'){
					auto lend = strchr(s + 1, ':');
					/* found ':' and length(left) > 0 and length(right) > 0 */
					if(!(!lend || (lend - (s + 1)) <= 0 || (lend + 1 == end))){
						auto rend = strchr(lend + 1, '%');
						if(rend){	/* matched syntax: %N?left:right% */
							if(p != q)
								fmtf[m++] = logtrans_fmt{p, q, -1};
							logtrans_match_arg arg0{s + 1, lend, lend + 1, rend};
							auto arg = (logtrans_match_arg * )malloc(sizeof(logtrans_match_arg));
							if(!arg) return -2;
							*arg = arg0;
							fmtf[m++] = logtrans_fmt{0, 0, i, arg, logtrans_match};
							p = rend + 1;
							q = rend;
							continue;
						}
					}
					/* syntax error: %N?left:right%, parse as %N instead */
				}
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
//			fprintf(stderr,"%zu, use index %d, fn=%p, arg=%p\n",i,fmtf[i].i, fmtf[i].fn, fmtf[i].arg);
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

static void pl_logtrans_free_format(logtrans_fmt * fmtf, size_t const & m)
{
	for(size_t i = 0; i < m; ++i){
		if(fmtf[i].arg)
			free(fmtf[i].arg);
	}
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
				if(fmtf[j].fn)
					fmtf[j].fn(arg[ii].first, arg[ii].second, fmtf[j].arg, base, len);
				else{
					if(arg[ii].first){
						base = arg[ii].first;
						len = arg[ii].second - arg[ii].first;
					}
					else{
						base = def;
						len = DEF_LEN;
					}
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
//	fprintf(stderr, "%s______________%s_______________\n", __FUNCTION__, fmt);
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
	pl_logtrans_free_format(fmtf, arn);
	return 0;
}
