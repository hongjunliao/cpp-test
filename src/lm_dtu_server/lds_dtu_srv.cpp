/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * dtu service
 */
#include "string_util.h"   /* str_t */
#include "lds_dtu_module.h"/* ModemDataStruct, ... */
#include "lds_inc.h"       /*  */
#include <stdio.h>
#include <string.h>        /* strchr */
#include <map>             /* std::map */

#define WQP_DELIM_CH ';'
#define MODEM_MAX 512
#define RECV_MAX (1024 * 1014 * 2)

/* lds_wqp_parse.cpp */
extern int lds_parse_wqp_log(char const * str, size_t len, std::map<char, str_t> & m);

/* buffer for receiving DTU data */
static char * recvbuf[MODEM_MAX] = { 0 };
static size_t recvbuf_len[MODEM_MAX] = { 0 };

int lm_dtu_recv_run()
{
	ModemDataStruct data;
	for(;;){
		data.m_modemId = 0;
		data.m_data_len = 0;

		/* read data from dtu, with no waiting */
		int r = lds_dtu_get_next_data(data, 0);
		if(r != 0)
			continue;

		if(data.m_modemId > MODEM_MAX){
			lds_log(LDS_LOG_DTU, LDS_LOG_DEBUG,"%s: modem_id too big, %u(max %d), skip \n",
					__FUNCTION__, data.m_modemId, MODEM_MAX);
			continue;
		}
		if(data.m_data_len <= 0){
			lds_log(LDS_LOG_DTU, LDS_LOG_DEBUG,"%s: %d, buffer empty, skip\n",
					__FUNCTION__, data.m_modemId);
			continue;
		}

		char *& recv_buf = recvbuf[data.m_modemId];
		size_t & len = recvbuf_len[data.m_modemId];

		if(!recv_buf)
			recv_buf = (char *)malloc(RECV_MAX);

		/* append to recv_buf */
		if(len >= RECV_MAX || len + data.m_data_len > RECV_MAX)
			len = 0;

		memcpy(recv_buf + len, data.m_data_buf, data.m_data_len);
		len += data.m_data_len;

		fprintf(stdout, "%s: id=%u, recv_buf='%s'\n", __FUNCTION__, data.m_modemId, recv_buf);

		/* parse recv_buf */
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
				lds_log(LDS_LOG_WQP, LDS_LOG_DEBUG,"%s: parse_wqp_log failed for '%s'\n", __FUNCTION__, p);
			}
			else{
				/* save to database*/

			}

			p = q + 1; /* to next wqp_log */
		}
		if(end){
			++end;
			len = recv_buf + len - end;
			memset(recv_buf, 0, end - recv_buf);
			memmove(recv_buf, end, len);
		}

		fprintf(stdout, "%s: id=%u, recv_buf(left)='%s'\n", __FUNCTION__, data.m_modemId, recv_buf);
	}
	return 0;
}
