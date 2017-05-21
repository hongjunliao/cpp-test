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
#include "lds_inc.h"              /* lds_log */
#include "lds_dtu_module.h"       /* lds_dtu_start_service, ... */
#include <stdio.h>

/* lds_option.cpp */
extern lds_options opt;
/* lds_dtu_srv.cpp */
extern int lm_dtu_recv_run();
/* tests
 * lds_test.cpp */
extern int test_lds_parse_wqp_log_main(int argc, char ** argv);

int test_lds_main(int argc, char ** argv)
{
//	test_lds_parse_wqp_log_main(argc, argv);

	int r = lds_parse_cmdline(argc, argv);
	char const * cmd = argc > 0? argv[0] : "lds_dtu_server";
	if(r != 0 || opt.help){
		lds_show_usage(cmd, stdout);
		return opt.help? 0 : -1;
	}
	if(opt.ver){
		lds_show_verison(cmd, stdout);
		return 0;
	}
	/* app init */
	r = lds_dtu_module_load("gprsdll.dll");
	if(r != 0){
		lds_log(LDS_LOG_APP, LDS_LOG_DEBUG, "%s: lds_dtu_module_load 'gprsdll.dll' failed\n", __FUNCTION__);
		return -1;
	}
	/* start dtu service */
	r = lds_dtu_start_service(opt.port);
	if(r != 0){
		lds_log(LDS_LOG_APP, LDS_LOG_DEBUG, "%s: DSStartService %d failed\n", __FUNCTION__, opt.port);
		return -1;
	}
	lds_log(LDS_LOG_APP, LDS_LOG_DEBUG, "%s: DSStartService at %d\n", __FUNCTION__, opt.port);

	/* run receive dtu data loop */
	lm_dtu_recv_run();

	/* stop dtu service */
	lds_dtu_stop_service();
	return 0;
}
