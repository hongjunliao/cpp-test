/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * dtu service
 */
#include "string_util.h"   /* str_t */
#include "lds_inc.h"       /*  */
#include <unistd.h>        /* sleep */
#include <stdio.h>
#include <string.h>        /* strchr */
#include <map>             /* std::map */

#define WQP_DELIM_CH ';'
/* buffer for receiving DTU data */
static char recv_buf[1024 * 1024 * 4] =
		"M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651;"
		"M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.652;haha;";

/* lds_wqp_parse.cpp */
extern int lds_parse_wqp_log(char const * str, size_t len, std::map<char, str_t> & m);
/* lds_main.cpp */
extern int (*gfn_lds_log)(int type, int level, char const * fmt, ...);;

int lm_dtu_recv_run()
{
	for(;;){
		fprintf(stdout, "%s: recv_buf='%s'\n", __FUNCTION__, recv_buf);
		/* read data from dtu */
		/* append to recv_buf */
		/* parse buf */
		size_t len = strlen(recv_buf);
		char * end = 0;
		for(char * p = recv_buf, * q = strchr(p, WQP_DELIM_CH);
				; q = strchr(p, WQP_DELIM_CH)){
			if(!q)
				break;
			end = q;

			std::map<char, str_t> wqplog;
			int r = lds_parse_wqp_log(p, q - p, wqplog);
			if(r != 0){
				*q = '\0';
				gfn_lds_log(LDS_LOG_WQP, LDS_LOG_DEBUG,"%s: parse_wqp_log failed for '%s'\n", __FUNCTION__, p);
			}
			else{
				/* save to database*/

			}

			p = q + 1; /* to next wqp_log */
		}
		if(end){
			++end;
			memset(recv_buf, 0, end - recv_buf);
			memmove(recv_buf, end, recv_buf + len - end);
		}

		fprintf(stdout, "%s: recv_buf(left)='%s'\n", __FUNCTION__, recv_buf);
		/* sleep a while and go to next loop */
		sleep(1);
	}
	return 0;
}
