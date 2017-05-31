/*!
 * 测试字符串处理算法
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * <剑指Offer>_2.3.4
 */
#include <stdio.h>
#include <string.h> /* strlen, strncat */
/*
 * 替换字符为指定字符串
 * replace every char @param c in @param str to @param r
 * @param buf: out
 * @param len: in, length for @param buf
 * @return:    return @param buf if success, else 0
 */
static char* replace_char_to_str(char const * str,
		char c, char const * r,
		char * buf, size_t len)
{
	auto end = str + strlen(str);
	if(!(str && str[0] != '\0' && r && r[0] != '\0' && buf && len > 0))
		return 0;

	size_t n = 0;
	auto rlen = strlen(r);
	for(auto p = str, q = p; ; ++q){
		if(*q == c || q == end){
			auto blen = q - p;
			if(blen > 0){
				if(n + blen > len - 1) /* for '\0' at end */
					return 0;

				memcpy(buf + n, p, blen);
				n += blen;
			}
			if(q == end)
				break;

			if(n + rlen > len - 1)
				return 0;
			strcat(buf + n, r);
			n += rlen;

			p = q + 1;
		}
	}
	buf[n] = '\0';

	return buf;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/* 键索引计数法: algs-5
 * LSD:低位优先的字符串排序法:运行时间与输入规模成正比
 * */
struct kic {
	char name[128];
	int key;		/* [0, R) */
};

static int test_kic_main(int argc, char ** argv)
{
	size_t const N = 64, R = 10;
	kic e[N], aux[N];
	int count[R];
	for(auto & i : e)
		++count[i.key + 1];
	for(size_t i = 0; i < R; ++i)
		count[i + 1] += count[i];
	for(size_t i = 0; i < N; ++i)
		aux[count[e[i].key]++] = e[i];

	return 0;
}

static int test_lsd_sort_main(int argc, char ** argv)
{
	size_t const W = 7;   /* length of string */
	size_t const N = 13;  /* size of elements */
	size_t const R = 256; /* alphabets */
	char * a[N] = { "4PGC938", "2IYE230", "3CI0720", "1ICK750", "10HV845", "4JZY524", "1ICK750", "3CIO720",
	"10HV845", "10HV845", "2rLA629", "2RLA629", "3ATW723"}, * aux[N];

	fprintf(stdout, "%s: sort before: [", __FUNCTION__);
	for(size_t i = 0; i < N; ++i)
		fprintf(stdout, "%s, ", a[i]);
	fprintf(stdout, "]\n");

	for(int d = W - 1; d >= 0; --d){
		int count[R + 1] = { 0 };
		for(size_t i = 0; i < N; ++i)
			++count[a[i][d] + 1];
		for(size_t r = 0; r < R; ++r)
			count[r + 1] += count[r];
		for(size_t i = 0; i < N; ++i)
			aux[count[a[i][d]]++] = a[i];
	}

	fprintf(stdout, "%s: sort after: [", __FUNCTION__);
	for(size_t i = 0; i < N; ++i)
		fprintf(stdout, "%s, ", aux[i]);
	fprintf(stdout, "\n");

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
int test_string_main(int argc, char ** argv)
{
	return test_lsd_sort_main(argc, argv);

	auto data = "we are happy";
	char str[128];
	auto s = replace_char_to_str(data, ' ', "%20", str, sizeof(str));

	if(!s)
		fprintf(stdout, "%s: replace_char_to_str failed\n", __FUNCTION__);
	else
		fprintf(stdout, "%s: replace_char_to_str, old='%s', char='%c', replace='%s', new='%s'\n",
				__FUNCTION__, data, ' ', "%20", s);

	test_kic_main(argc, argv);
	return 0;
}
