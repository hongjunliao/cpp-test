/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 *
 * process wqp save format
 *
 */
#include "lds_wqp.h"       /* lds_wqp */
#include "lds_inc.h"
#include "string_util.h"   /* sdump_str */
#include <stdlib.h>
#include <string.h>        /* strlen */

char * lds_sprintf_wqp(char * buf, size_t sz, char const * fmt, lds_wqp & m)
{
	char dfmt[strlen(fmt) * 2];
	lds_log(LDS_LOG_FMT, LDS_LOG_DEBUG, "%s: fmt='%s'\n", __FUNCTION__, sdump_str(dfmt, fmt, strlen(fmt)));

	if(!(buf && sz > 0 && fmt && fmt[0] != '\0'))
		return 0;

	size_t len = strlen(fmt);
	if(sz < len)
		return 0;

	if(len == 1){
		buf[0] = fmt[0];
		buf[1] = '\0';
		return buf;
	}

	size_t j  = 0;
	for(size_t i = 0; i < len; ++i){
		if( i < len - 1){
			char c = fmt[i + 1];
			if(fmt[i] == '$' && (c >= 'A' && c <= 'Z')){
				strcpy(buf + j, m[c].val);
				j += strlen(m.at(c).val);
				++i;
				continue;
			}
		}
		buf[j++] = fmt[i];
	}
	buf[j] = '\0';
	lds_log(LDS_LOG_FMT, LDS_LOG_DEBUG, "%s: result='%s'\n", __FUNCTION__, buf);

	return buf;
}
