/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * test
 */

#ifdef DEBUG

#include "string_util.h"   /* str_t */
#include <stdio.h>
#include <string.h>        /* strlen */
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

