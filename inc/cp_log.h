/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/27
 *
 * */
#ifndef CPP_TEST_H_
#define CPP_TEST_H_
#include <stdio.h>      /* fprintf */

/////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#define cp_fprintfc(c, f, fmt, args...)                                      \
	fprintf((f), "\x1b[%dm" fmt "\e[0m"                                      \
    , ((c) >= 6? 35 : ((c) == 5? 31 : ((c) == 4? 33 : 0)))                   \
    , ##args)

#define cp_printfc(c, fmt, args...)                                          \
	cp_fprintfc(((c) >= 0? (c) : 5), (((c) > 3 || (c) < 0)? stderr : stdout), fmt, ##args)

#define cp_fprintf(f, fmt, args...)                                          \
	cp_fprintfc((stderr == (f)? 4: 0), (f), fmt, ##args)

#define cp_log(c, fmt, args...)                                              \
   cp_printfc((c), "[%s][%s:%d] %s: " fmt                               \
    , (((c) >= 6? "FATAL" : ((c) == 5? "ERROR" : ((c) == 4? "WARNG" :        \
      ((c) == 3? "INFOO" : ((c) == 2? "DEBUG" : "TRACE"))))))                \
   , __FILE__, __LINE__, __FUNCTION__, ##args)

/* a simple log system using fprintf, with colorful output
 * sample log:
 * [FATAL][../src/cp_test.cpp:26] cp_test_main: fatal log, msg='fatal', value=10
 * [ERROR][../src/cp_test.cpp:27] cp_test_main: error log, msg='error', value=10
 * [WARNG][../src/cp_test.cpp:28] cp_test_main: warning log, msg='warning', value=10
 * [INFOO][../src/cp_test.cpp:29] cp_test_main: info log, msg='info', value=10
 * [DEBUG][../src/cp_test.cpp:30] cp_test_main: debug log, msg='debug', value=10
 * [TRACE][../src/cp_test.cpp:31] cp_test_main: trace log, msg='trace', value=10
 *  */
#define cp_logf(fmt, args...) cp_log(6, fmt, ##args)
#define cp_loge(fmt, args...) cp_log(5, fmt, ##args)
#define cp_logw(fmt, args...) cp_log(4, fmt, ##args)
#define cp_logi(fmt, args...) cp_log(3, fmt, ##args)
#define cp_logd(fmt, args...) cp_log(2, fmt, ##args)
#define cp_logt(fmt, args...) cp_log(1, fmt, ##args)

#ifdef __cplusplus
}
#endif

#endif /* CPP_TEST_H_ */
