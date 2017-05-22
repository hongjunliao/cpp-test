/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 *
 * water quality parameters/wqp parse
 *
 */
#include "lds_wqp.h"   /*  */
#include "lds_inc.h"
#include <stdlib.h>    /* strtol */
#include <string.h>    /* strncpy */
#include <time.h>      /* time_t */

#define IS_CHAR_A_TO_Z(ch) ((ch) >= 'A' && (ch) <= 'Z')

/* 水质参数 */
struct wqp_vaal {
	int mid;                              /* 监控点ID */
	time_t theTime_;                      /* 时间 */
	char runStatus;                       /* 运行状态 */
	char remark_;                         /* 备注 */
	float devroom_tem, devroom_hum;       /* 温度, 湿度 */
	float a, ph, orp, do_, sd;            /* A, PH值, ORP, DO, SD */
	float cod, nh, chla, tp, water_tem;   /* COD, NH, CHLA, TP, 水温 */
	float level1, level2, level3, level4; /* 液位 */
	char za1, za2;                        /* 闸门 */
};

#define convert_wqp_val_chr(buf, c) { buf[0] = c; buf[1] = '\0'; }

static void convert_wqp_val(char * buf, char c, char const * str, size_t len)
{
	if(!(buf && str && len > 0))
		return;

	strncpy(buf, str, len);

	char * s, * end = buf + len;
	switch(c){
	case 'M':
		int r = strtol(buf, &s, 10);

		break;
	}
	buf[0] = '2';
	buf[1] = '\0';
}

int lds_parse_wqp(char * str, size_t len, lds_wqp & m)
{
	if(!(str && str[0] != '\0' && len > 0))
		return -1;

	for(char * q = str; q != str + len; ){
		if(IS_CHAR_A_TO_Z(*q)){
			char * p = q + 1;

			if(p == str + len)
				return 0;

			wqp_val & val = m[*q];
			if(IS_CHAR_A_TO_Z(*p)){ /* wqp_val is only a char */
				convert_wqp_val_chr(val.val, *p);
				q +=2;
				continue;
			}
			/* move to next wqp_tag */
			do {
				++p;
			}
			while(p != str + len && !IS_CHAR_A_TO_Z(*p));

			convert_wqp_val(val.val, *q, q + 1, p - (q + 1));	/* p - (q + 1) >= 1 */

			if(p == str + len)
				return 0;

			q = p; /* move to next wqp_log */
			continue;
		}
		return -1;
	}
	return 0;
}
