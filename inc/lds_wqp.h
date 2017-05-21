/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/21
 *
 * water quality parameters/wqp value
 *
 */
#include "string_util.h"   /* str_t */
#include <map>             /* std::map */

/* max length for wqp_val, currently time(which tag is T) the max */
#define WQP_MAX 16

struct wqp_val {
	char val[WQP_MAX];
};
/* map<wqp_tag, wqp_val> */
typedef std::map<char, wqp_val> lds_wqp;
/* parse @param str as wqp_log
 * sample: 'M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651'
 *
 * @param str:  wqp_log
 * @param len:  length for @param str
 * @param m:    lds_wqp
 *
 * @return: 0 on success
 * */
extern int lds_parse_wqp(char * str, size_t len, lds_wqp & m);
extern int lds_send_wqp(char const * str);

extern char * lds_sprintf_wqp(char * buf, size_t len, char const * fmt, lds_wqp & m);
