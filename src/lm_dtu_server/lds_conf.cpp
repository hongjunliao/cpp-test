/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun,$liao <docici@,$com>, @date ,$/,$/,$
 *
 * process wqp save format
 *
 */
#include "lds_inc.h"
#include "process_util.h"  /* intercmdctx */
#include <stdio.h>

/* lds_wqp_send.cpp */
extern intercmdctx gcmdctx;
/* lds_main.cpp */
extern char g_root_dir[MAX_PATH * 3];

/* load config file */
int lds_load_conf(char const * f)
{
	gcmdctx.dir = g_root_dir;
	gcmdctx.cmd = "sqlcmd -u -S 121.37.60.39,2073 -P pwqp43we9_45fn320vgd_irb_i92p6hge3_873w98jmwutc5602301"
			" -U pooi9008_fn320vgd_irb_i92p6hge3_873w98jmwutc56023vfi8";
	lds_log(LDS_LOG_CNF, LDS_LOG_DEBUG, "%s: dir='%s',cmd='%s'\n",
			__FUNCTION__, gcmdctx.dir, gcmdctx.cmd);
	return -1;
}
