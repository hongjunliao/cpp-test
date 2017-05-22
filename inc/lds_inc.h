/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * include file
 */
#ifndef LDS_INC_H_
#define LDS_INC_H_

#include <stdio.h>	/* FILE* */
/* log type */
#define LDS_LOG_APP 0
#define LDS_LOG_MEM 1 /* 表示该条日志是跟系统内存有关的,如分配内存失败  */
#define LDS_LOG_WQP 2 /* 表示该条日志是跟水质数据有关的 */
#define LDS_LOG_DTU 3 /* DTU */
#define LDS_LOG_FMT 4 /* wqp directive format */
#define LDS_LOG_TST 5 /* test */
/* TODO: change this value when you add new log_type */
#define LDS_LOG_TYPE_MAX (LDS_LOG_TST + 1)

/* log level */
#define LDS_LOG_INFO  0
#define LDS_LOG_DEBUG 1
#define LDS_LOG_ERR   2
/* TODO: change this value when you add new log_level */
#define LDS_LOG_LEVEL_MAX (LDS_LOG_ERR + 1)

/* log system
 * log formt: '[datetime] log_type/log_level: <log contents>'
 * */
extern int lds_log(int type, int level, char const * fmt, ...);

/* cmdlime and options */
struct lds_options {
	int port;          /* DTU服务监听端口号 */
	char const * sav;  /* 发送到sqlcmd的指今格式 */
	int ver;           /* show version info? */
	int help;          /* show help info? */
};

/*
 * 命令行参数解析
 * NOTE: 参数和值之间请不要有空白符, 如: -p8010 */
extern int lds_parse_cmdline(int argc, char ** argv);

extern void lds_show_usage(char const * cmd, FILE * f);
extern void lds_show_verison(char const * cmd, FILE * f);

/* config file */
extern int lds_load_conf(char const * f);
#endif /* LDS_INC_H_ */
