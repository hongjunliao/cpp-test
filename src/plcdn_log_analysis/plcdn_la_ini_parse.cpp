/*!
 * This file is PART of plcdn_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2017/03/30
 *
 * ini config file parse, syntax sample:
 *
 * #this is a commnet
 * [section1]
 * key=value
 * [section2]
 * key2 = value2
 */
#include <stdio.h>
#include <ctype.h>	/* isblank */
#include <string.h>	/* strrchr */

struct strl_t{
	char const * data;
	size_t len;
};

/* @param buf must be trimed for '\r\n\t ' */
int parse_ini(char const * buf, strl_t * comment, strl_t * section, strl_t * key, strl_t * value)
{
	auto end = buf + strlen(buf);

	auto p = &buf[0];
	if(*p == '#'){	/* comments */
		comment->data = p + 1;
		comment->len = end - comment->data;
		return 2; /* begin with '#', commnets */
	}

	if(*p == '['){ /* begin with '[', section */
		auto c = strrchr(p + 1, ']');
		if(c){
			for(auto q = p + 1; q != c; ++q){
				if(*q == '[' || *q == ']')
					return -1;	/* syntax error: '[' or  ']' between [ and ] not allowed */
			}

			for(auto q = c + 1; q != end; ++q){
				if(!isblank(*q))
					return -1; /* syntax error: characters after ] not allowed */
			}

			section->data = p + 1;
			section->len = c - section->data;
			return 3;	/* [x], section */
		}
	}

	if(*p == '='){	/* maybe key-value */
		/* blanks in key not allowed */

//		key->data = s;
		key->len = p - key->data;

		value->data = p + 1;
		value->len = end - key->data;
		return 4;
	}

	return -1;
}


int ini_read_file(char const * f);


