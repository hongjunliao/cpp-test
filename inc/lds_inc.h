/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * include file
 */

/* log type */
#define LDS_LOG_APP 0
#define LDS_LOG_MEM 1
#define LDS_LOG_WQP 2
/* TODO: change this value when you add new log_type */
#define LDS_LOG_TYPE_MAX (LDS_LOG_WQP + 1)
/* log level */
#define LDS_LOG_INFO  0
#define LDS_LOG_DEBUG 1
#define LDS_LOG_ERR   2
/* TODO: change this value when you add new log_level */
#define LDS_LOG_LEVEL_MAX (LDS_LOG_ERR + 1)

/* log system */
int lds_log_to_file(int type, int level, char const * fmt, ...);

/* FIXME: C99 only */
#define lds_log(fmt, args)   lds_log_to_file(LDS_LOG_APP, LDS_LOG_INFO, fmt, ##__VA_ARGS__)
