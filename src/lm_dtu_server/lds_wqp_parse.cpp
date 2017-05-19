/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 *
 * water quality parameters/wqp parse
 *
 */
#include "string_util.h"   /* str_t */
#include <map>             /* std::map */

#define IS_CHAR_A_TO_Z(ch) ((ch) >= 'A' && (ch) <= 'Z')

/* parse @param str as wqp_log
 * sample: 'M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651'
 * @param delim: delimer for wqp_log
 * @param m: map<wqp_tag, wqp_val>
 * @return: 0 on success
 * */

int lds_parse_wqp_log(char const * str, size_t len, std::map<char, str_t> & m)
{
	if(!(str && str[0] != '\0' && len > 0))
		return -1;

	for(char const * q = str; q != str + len;){
		if(IS_CHAR_A_TO_Z(*q)){
			char const * p = q + 1;

			if(p == str + len)
				return -1;	/* syntax error */

			m[*q].beg = (char *)p;

			if(IS_CHAR_A_TO_Z(*p)){ /* wqp_val is only a char */
				m[*q].end = (char *)p + 1;
				q +=2;
				continue;
			}
			/* move to next wqp_tag */
			do {
				++p;
			}
			while(p != str + len && !IS_CHAR_A_TO_Z(*p));

			m[*q].end = (char *)p;

			q = p; /* move to next wqp_log */
			continue;
		}
		return -1;
	}
	return 0;
}
