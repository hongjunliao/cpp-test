/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun,$liao <docici@,$com>, @date ,$/,$/,$
 *
 * process wqp save format
 *
 */
#include "lds_wqp.h"   /* lds_wqp */
#include "lds_inc.h"
#include "string_util.h"   /* str_t */
#include <stdlib.h>
#include <string.h>        /* strlen */
/*
 * sample: "exec sp_insert_wpp $M,$S,$R,$T,$Y,$Z,$A,$B,$C,$D,$E,$F,$G"
 * */
char * lds_sprintf_wqp(char * buf, size_t len, char const * fmt, lds_wqp & m)
{
	if(!(buf && len > 0 && fmt && fmt[0] != '\0'))
		return 0;

	size_t j  = 0;
	for(size_t i = 0; i != len - 1; ++i){
		char c = fmt[i + 1];
		if(fmt[i] == '$' && (c >= 'A' && c <= 'Z')){
			if(m.count(c) > 0){
				strcpy(buf + j, m.at(c).val);
				j += strlen(m.at(c).val);

				++i;
			}
			else
				buf[j++] = '\0';
			continue;
		}
		buf[j++] = fmt[i];
	}
	return buf;
}
