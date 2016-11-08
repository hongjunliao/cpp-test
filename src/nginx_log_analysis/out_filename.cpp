/*!
 * This file is PART of nginx_log_analysis
 * parse formats in output filename,
 * e.g. "-r ipsource.${datetime}.${device_id}", "-u urlstat.${datetime}.${device_id}.${site_id}"
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 */
#include <stdio.h>
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 	/*std::string*/
//
//#include <fnmatch.h>	/*fnmatch*/
//#include <sys/sysinfo.h>	/*get_nprocs*/
//#include <sys/stat.h>	/*fstat*/
//#include <sys/mman.h>	/*mmap*/
////#include <locale.h> 	/*setlocale*/
//#include <pthread.h> 	/*pthread_create*/
//#include <map>				/*std::map*/
////#include <thread>		/*std::thread*/
////#include <atomic>		/*std::atomic*/
//#include "bd_test.h"		/*test_nginx_log_stats_main*/
//#include "test_options.h"	/*nla_options**/
//#include "nginx_log_analysis.h"	/*log_stats, ...*/
//#include "string_util.h"	/*md5sum*/
//#include "net_util.h"	/*get_if_addrs, ...*/
//#include <algorithm>	/*std::min*/

//static void arg_add(char const * key, char const * val){
//	static std::unordered_map<std::string, std::string> arg_map;
//	arg_map[key] = val;
//}
//
//int parse_str_with_var(char const * str)
//{
//	if(!str || str[0] == '\0') return -1;
//
//	return 0;
//}
