/*!
 * This file is PART of nginx_log_analysis
 * rotate nginx log
 * @author hongjun.liao <docici@126.com>
 * @date 2017/02
 */
#include <cstdio>
#include <cstring>			/* memset */
#include <ctime>			/* time_t */
#include <unordered_map> 	/*std::unordered_map*/
#include <string> 			/*std::string*/
#include "test_options.h"		/*plcdn_la_options*/
#include "nginx_log_analysis.h"	/*nginx_domain_stat, ...*/

/*main.cpp*/
extern time_t g_plcdn_la_start_time;

/*plcdn_log_analysis/option.cpp*/
extern struct plcdn_la_options plcdn_la_opt;

int nginx_rotate_log(char const * rotate_dir, FILE * logfile,
		std::unordered_map<std::string, nginx_domain_stat> const& stats)
{
	if(!rotate_dir || rotate_dir[0] == '\0')
		return -1;
	if(!logfile && plcdn_la_opt.verbose > 3){
		fprintf(stderr, "%s: logfile NULL, analysis rotate dir only\n", __FUNCTION__);
	}
    log_item item;
    char buf[8192];
    while (fgets(buf, sizeof(buf), logfile)){
    	char * p = buf;
    	memset(&item, 0, sizeof(log_item));
    	item.beg = p;
    	char *items[18];
    	int result = do_parse_nginx_log_item(items, p, '\n');
    	if(result != 0){
    		return 1;
    	}
    	item.end = p;
    }
	return 0;
}
