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

int test_string_main(int argc, char ** argv)
{
	auto data = "we are happy";
	char str[128];
	auto s = replace_char_to_str(data, ' ', "%20", str, sizeof(str));

	if(!s)
		fprintf(stdout, "%s: replace_char_to_str failed\n", __FUNCTION__);
	else
		fprintf(stdout, "%s: replace_char_to_str, old='%s', char='%c', replace='%s', new='%s'\n",
				__FUNCTION__, data, ' ', "%20", s);
	return 0;
}
