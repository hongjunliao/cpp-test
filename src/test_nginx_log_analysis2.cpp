/*!
 * TEST for nginx_log_analysis
 */
#include <sys/sysinfo.h>	/*get_nprocs*/
#include <sys/resource.h>	/*setpriority*/
#include <unistd.h>	/*getpid*/
#include <time.h>	/*time_t, strptime*/
#include <stdio.h>
#include <string.h> /*strncpy*/
#include <math.h> /*pow*/
#include <assert.h>
#include <pthread.h> /*pthread*/

#include <vector>
#include <algorithm> /*std::sort*/
#include <unordered_map> /*unordered_map*/
#include <map>
#include "bd_test.h"	/*test_nginx_log_split_main*/
#include "nginx_log_analysis.h"	/*user-defined struts, log_stats, ...*/
#include "string_util.h"	/*strlwr*/
#include "test_options.h"	/*nla_options**/
/*tests, @see do_test*/
static int test_strptime_main(int argc, char ** argv);
static int test_time_mark_main(int argc, char ** argv);
int test_nginx_log_split_main(int argc, char ** argv);
/*test_mem.cpp*/
extern int test_alloc_mmap_main(int argc, char * argv[]);
/*net_util.cpp*/
extern int test_net_util_main(int argc, char ** argv);

/*all options: test_options.cpp*/
extern struct nla_options nla_opt;

bool compare_by_access_count(url_count const& a, url_count const& b)
{
	return a.count > b.count;
}
static void url_top_n(std::map<time_group, log_stat> const& stats, std::vector<url_count>& urlcount)
{
	std::unordered_map<std::string/*char const **/, url_stat> urlstats;
	for(auto it = stats.begin(); it != stats.end(); ++it){
		auto const& urlstat = it->second._url_stats;
		for(auto const& item : urlstat){
			urlstats[item.first] += item.second;
		}
	}
	for(auto const& item : urlstats){
		url_count c = {item.first, item.second.access_total()};
		urlcount.push_back(c);
	}
	std::sort(urlcount.begin(), urlcount.end(), compare_by_access_count);
}


static int test_time_mark_main(int argc, char ** argv)
{
	char const * stime = "17/Sep/2016:01:19:43";
	time_t time1 = time(NULL);
	tm time2 = *localtime(&time1);
	time2.tm_hour -= 1;
	assert((time1 - 3600) == mktime(&time2));
	fprintf(stdout, "time=%ld, difftime(0, time)=%ld\n", time1, (long)difftime((time_t)0, time1));

	time_group timem1(stime), timem2(stime);
	assert(timem1 == timem2);
	assert(timem1 == time_group("17/Sep/2016:01:19:00"));
	fprintf(stdout, "time=17/Sep/2016:01:19:23,mark=%s\n", time_group("17/Sep/2016:01:19:23").c_str());
	fprintf(stdout, "time=%s, start=%s\n", stime, timem1.c_str());
	for(int i= 0; i < 5; ++i){
		fprintf(stdout, "next=%s\n", timem1.next().c_str());
		timem1 = timem1.next();
	}
	time_group startm("17/Sep/2016:01:19:43"), timem3(startm);
	assert(startm == time_group("17/Sep/2016:01:15:43"));
	return 0;
}
static int test_strptime_main(int argc, char ** argv)
{
	fprintf(stdout, "%s:\n", __FUNCTION__);
	struct tm my_tm;
	char const * stime = "17/Sep/2016:08:19:43";
	char const * result = strptime(stime, "%d/%b/%Y:%H:%M:%S" , &my_tm);
	my_tm.tm_isdst = 0;
	mktime(&my_tm);
	fprintf(stdout, "time=%s, NOT paused=[%s], asctime:sec=%d,all=%s\n",
			stime, result? result : "<null>", my_tm.tm_sec, asctime(&my_tm));
	return 0;
}

int test_nginx_log_split_main(int argc, char ** argv)
{
	if(argc < 3){
		fprintf(stderr, "split nginx log file into multiple files, by domain field.\n"
				"usage: %s <nginx_log_file> <output_folder>\n"
				"  <nginx_log_file>     nginx log file\n"
				"  <output_folder>      folder name where splitted files will save to\n"
				, __FUNCTION__);
		return 1;
	}
	FILE * f = fopen(argv[1], "r");
	if(!f) {
		fprintf(stderr, "fopen file %s failed\n", argv[1]);
		return 1;
	}
	char out_folder[32] = "logs";
	strcpy(out_folder, argv[2]);
	if(out_folder[strlen(out_folder) -1 ] != '/')
		strcat(out_folder, "/");

	std::unordered_map<std::string, FILE *> dmap;/*domain : log_file*/
	size_t linecount = 0;
	char data[8192] = "";
	char const * result = 0;
	while((result = fgets(data, sizeof(data), f)) != NULL){
		++linecount;
		fprintf(stdout, "\rprocessing %8ld line ...", linecount);
		int len = strlen(result);
		if(result[len - 1] != '\n'){
			fprintf(stderr, "\n%s: WARNING, length > %zu bytes, skip:\n%s\n", __FUNCTION__, sizeof(data), data);
			continue;
		}
		char domain[128] = "", out_file[512] = "";
		strncpy(domain, data, strchr(data, ' ') - data);
		strlwr(domain);

		FILE * & file = dmap[domain];
		sprintf(out_file, "%s%s", out_folder, domain);
		if(!file && (file = fopen(out_file, "w")) == NULL){
			fprintf(stderr, "\n%s: WARNING, create file %s failed, skip\n", __FUNCTION__, domain);
			continue;
		}
		int result = fwrite(data, sizeof(char), len, file);
		if(result < len || ferror(file)){
			fprintf(stderr, "\n%s: WARNING, write domain file %s NOT complete:\n%s\n", __FUNCTION__, domain, data);
		}
	}
	fprintf(stdout, "\n");
	return 0;
}


int test_nginx_log_analysis_main(int argc, char ** argv)
{
	//	test_strptime_main(argc, argv);
	//	test_time_mark_main(argc, argv);

		/*siteuidlist.txt www.haipin.com*/
	//	fprintf(stdout, "%s: file=%s, site=%s, id=%d\n", __FUNCTION__, argv[1], argv[2], find_site_id(argv[1], argv[2]));

	//	test_get_if_addrs_main(argc, argv);
	//	test_alloc_mmap_main(argc, argv);
	//	fprintf(stdout, "done, press anykey to exit.");
	//	getc(stdin);

//		char str1[] = "hello", str2[] = "jack", str3[] =  "hello";
//		fprintf(stdout, "%s: test str_find()\n"
//				"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n"
//				"\tstr=%s, addr=%p, find=%p\n\tstr=%s, addr=%p, find=%p\n", __FUNCTION__,
//				str1, str1, str_find(str1), str2, str2, str_find(str2),
//				str3, str3, str_find(str3), str2, str2, str_find(str2));

//	char str1[] = "hello", str2[] = "HELLO";
//	fprintf(stdout, "strlupr(): str=%s, strupr=%s\n, strlwr(): str=%s, strulwr=%s\n",
//			"hello", strupr(str1), "HELLO", strlwr(str2));
//	int core_num = get_nprocs();
//	fprintf(stdout, "%s: core_num=%d\n", __FUNCTION__, core_num);
//	int term_cols = 0, term_rows = 0;
//	termio_get_col_row(&term_cols, &term_rows);
//	fprintf(stdout, "term_cols = %d, term_rows = %d\n", term_cols, term_rows);
//	test_net_util_main(argc, argv);
	//
	//	fprintf(stream, "%-20s%-120s%-10s%-20s\n", "Time", "Url", "Count", "Bytes");
	//	size_t bytes_total = 0, bytes_200 = 0, bytes_206 = 0, other_bytes = 0;
	//	size_t access_total = 0, access_200 = 0, access_206 = 0;
	//	for(auto it = stats.begin(); it != stats.end(); ++it){
	//		log_stat const& stat = it->second;
	//		auto total = stat.bytes_total(), b200 = stat.bytes(200), b206 = stat.bytes(206);
	//		bytes_total += total;
	//		bytes_200 += b200;
	//		bytes_206 += b206;
	//		other_bytes += (total - b200 - b206);
	//		access_total += stat.access_total();
	//		access_200 += stat.access(200);
	//		access_206 += stat.access(206);
	//
	////		fprintf(stream, "%-20s%-120s%-10s%-20.0f\n", it->first.c_str(), " ", " ", stat.bytes_total());
	////		if(top > 0 && --top <= 0)
	////			break;
	////
	////		for(auto it2 = stat._url_stats.begin(); it2 != stat._url_stats.end(); ++it2){
	////			fprintf(stream, "%-20s%-120s%-10ld%-20s\n", " ", it2->first, it2->second.access_total(), " ");
	////		}
	//	}
	//	fprintf(stream, "%-15s%-15s%-20s%-20s%-20s%-20s%-20s%-40s\n", "access_total", "access_200", "access_206",
	//			"bytes_200", "bytes_206", "200+206", "other", "Total Byte/GB");
	//	fprintf(stream, "%-15ld%-15ld%-20ld%-20zu%-20zu%-20zu%-20zu%-20zu/%-20.6f\n",
	//			access_total, access_200, access_206, bytes_200,
	//			bytes_206, bytes_200 + bytes_206, other_bytes, bytes_total, bytes_total / pow(1024.0, 3));
	//
	//	std::vector<url_count> urlcount;
	//	url_top_n(stats, urlcount);
	//	fprintf(stream, "%-100s%-70s\n", "url", "count");
	//	int left = 10;
	//	for(auto it = urlcount.begin(); it != urlcount.end() && left > 0; ++it){
	//		fprintf(stream, "%-100s%-70ld\n", it->url, it->count);
	//		--left;
	//	}
	//	return 0;

	int r = setpriority(PRIO_PROCESS, getpid(), -10);
	if(r !=0 && nla_opt.verbose){
		fprintf(stderr, "%s: setpriority failed\n", __FUNCTION__);
	}
	return 0;
}
