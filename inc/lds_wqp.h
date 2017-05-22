/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/21
 *
 * water quality parameters/wqp value
 *
 */
#include "string_util.h"   /* str_t */
#include "process_util.h"  /* intercmdctx */
#include <map>             /* std::map */

/* max length for wqp_val, currently time(which tag is T) the max */
#define WQP_MAX 16

/* 水质参数的值, 该值是经过从水质参数日志中解析及转换过后的 */
struct wqp_val {
	char val[WQP_MAX];
};
/* map<wqp_tag, wqp_val> */
typedef std::map<char, wqp_val> lds_wqp;
/* parse @param str as wqp_log
 * 解析水质参数, 相应wqp_tag的值被保存到wqp_val, 如果值有误, 根据wqp_tag的不同, 可能为默认值;
 * 有些wqp_tag的值必须是正确的, 如监控点ID号, 时间
 * sample: 'M005S0RAT120820063133Y1Z0A202.6B8.24C1.33D0.00E29.5F76.3G4.651'
 *
 * @param str:  wqp_log
 * @param len:  length for @param str
 * @param m:    lds_wqp
 *
 * @return: 0 on success
 * */
extern int lds_parse_wqp(char * str, size_t len, lds_wqp & m);

/*
 * 发送指定到@param ctx, @see intercmdctx;如发送SQL insert语句到sqlcmd的标准输入,
 * sqlcmd将会执行该语句,通过此方式从而达到保存水质数据到数据库的目的
 * @param str:  指令/directive, 如SQL insert语句
 * @return:     0 on success
 *
 * @note:
 * (1)what is sqlcmd? see https://docs.microsoft.com/en-us/sql/tools/sqlcmd-utility
 * (2)这只是其中一种将水质数据保存到数据库的方式, 这种方式目前有BUG, 如在捕获指令执行结果方面存在问题,
 *    从而不能准确得知指令是否执行正确, @see send_intercmd, 你也可以直接使用ADO进行数据库访问, 并将@param str 转给相应Ado对象：
 *    https://docs.microsoft.com/en-us/sql/ado/reference/ado-api/execute-requery-and-clear-methods-example-vc
 * @return: 如果该指令被正确发送到@param ctx了, return 0
 * */
extern int lds_send_wqp(intercmdctx const& ctx, char const * str);

/*
 * printf-style format function
 * 按@param @fmt中指定的格式将水质参数格式化到字符串@param buf; 语法:
 * $<wqp_tag> 表示引用相应字段值, 如'$T'表示引用时间, '$B'表示引用PH值
 * param buf:  buffer for format string
 * @param len: length for @param buf, make sure big enough
 * @param fmt: sample: "exec sp_insert_wpp $M,$S,$R,$T,$Y,$Z,$A,$B,$C,$D,$E,$F,$G\ngo\n"
 * @param m:   lds_wqp, 如果某个wqp_tag无值, 在格式化时将被空值替代,这可能会造成生成的格式化字符串有语法错误
 *
 * @return:    返回格式化后的字符串, 即@apram buf, NULL if has errors
 */
extern char * lds_sprintf_wqp(char * buf, size_t len, char const * fmt, lds_wqp & m);
