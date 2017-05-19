/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * main
 *
 * 水质参数:    wqp/water quality parameters, sample: 水的PH值
 * DTU:        data transfer unit, 一种无线数据传输设备
 * 水质参数日志: wqp_log, 格式: 由标签及水质参数值或其它数据(如时间)组成
 * (1)标签/wqp_tag:  一个大写字母 A-Z
 *　(2)水质参数值/wqp_val: 可以是int, float,　char
 *
 * 水质参数日志示例: 'M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651'
 */
#include "string_util.h"   /* str_t */
#include <stdio.h>
#include <string.h>        /* strlen */
#include <map>             /* std::map */

/* log */
int (*gfn_lds_log)(int type, int level, char const * fmt, ...);

/* lds_wqp_parse.cpp */
extern int lds_parse_wqp_log(char const * str, size_t len, std::map<char, str_t> & m);
/* lds_dtu_srv.cpp */
extern int lm_dtu_recv_run();
/* lds_log.cpp */
extern int lds_log_to_file(int type, int level, char const * fmt, ...);

static int test_lds_parse_wqp_log_main(int argc, char ** argv)
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

int test_lds_main(int argc, char ** argv)
{
//	test_lds_parse_wqp_log_main(argc, argv);
	/* app init */
	gfn_lds_log = lds_log_to_file;
	/* start dtu service */
	/* run receive dtu data loop */
	lm_dtu_recv_run();

	/* stop dtu service */
	return 0;
}
