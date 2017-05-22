/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * test
 * NOTE: 本文件的内容仅供测试用
 */
#include "string_util.h"   /* sdump_str */
#include "lds_wqp.h"      /* lds_sprintf_wqp */
#include "lds_inc.h"
#include <string.h>        /* strlen */

#ifdef DEBUG

#include <stdio.h>
#include <map>             /* std::map */

int test_lds_parse_wqp_log_main(int argc, char ** argv)
{
	char const * str = "M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651";

	std::map<char, str_t> wqplog;
	int r = lds_parse_wqp_log(str, strlen(str), wqplog);
	if(r != 0){
		fprintf(stdout, "%s: parse_wqp_log failed\n", __FUNCTION__);
		return -1;
	}

	fprintf(stdout, "%s: data='%s', parse results:\n", __FUNCTION__, str);
	for(std::map<char, str_t>::iterator it = wqplog.begin(); it != wqplog.end(); ++it){
		fprintf(stdout, "%s: %c=", __FUNCTION__, it->first);
		str_t_fprint(&it->second, stdout);
	}

	return 0;
}
#endif /* DEBUG */

int test_lds_sprintf_wqp(int argc, char ** argv)
{
	char const * fmt = "exec sp_insert_wpp $M,$S,$R,$T,$Y,$Z,$A,$B,$C,$D,$E,$F,$G\ngo\n";
	char direct[1024];
	lds_wqp wqplog;
	fprintf(stdout, "%s: _____K='%s'____\n", __FUNCTION__, wqplog['K'].val);

	strcpy(wqplog['M'].val, "1.1");
	strcpy(wqplog['A'].val, "2.2");
	strcpy(wqplog['G'].val, "3.3");
	char const * str = lds_sprintf_wqp(direct, sizeof(direct), fmt, wqplog);
	if(!str){
		lds_log(LDS_LOG_TST, LDS_LOG_DEBUG,"%s: lds_sprintf_wqp failed, fmt='%s'\n",
				__FUNCTION__, fmt);
		return -1;
	}

	lds_log(LDS_LOG_TST, LDS_LOG_DEBUG,"%s: fmt='%s', result='%s'\n",
			__FUNCTION__, fmt, str);

	return 0;
}
