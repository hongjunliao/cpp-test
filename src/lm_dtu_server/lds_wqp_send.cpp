/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/21
 *
 * water quality parameters/wqp value
 *
 */
#include "process_util.h"  /* send_intercmd */
#include "lds_inc.h"

int lds_send_wqp(char const * str)
{
	if(!(str && str[0] != '\0'))
		return -1;
	lds_log(LDS_LOG_WQP, LDS_LOG_INFO, "%s: direct='%s'\n", __FUNCTION__, str);

	intercmdctx ctx = { 0 };
	ctx.dir = "D:/ws/release/";
	ctx.cmd = "sqlcmd -u -S 121.37.60.39,2073 -P pwqp43we9_45fn320vgd_irb_i92p6hge3_873w98jmwutc5602301"
			" -U pooi9008_fn320vgd_irb_i92p6hge3_873w98jmwutc56023vfi8";
	ctx.direct = str;

	char buf[1024 * 10];
	int r = send_intercmd(ctx, buf, sizeof(buf), 0, 0);
	if(r != 0){
		lds_log(LDS_LOG_WQP, LDS_LOG_ERR, "%s: send_intercmd failed, direct='%s'\n", __FUNCTION__, str);
		return -1;
	}

	return 0;
}
