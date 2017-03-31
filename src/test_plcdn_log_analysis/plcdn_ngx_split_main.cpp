/*!
 * This file is PART of nginx_log_split
 * split nginx logs by domain, into multi-log-files, each log is "single-domained", used by nginx_log_analysis
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 * 2.gcc enable c++11: -std=c++0x
 * 3.gcc include path add -I../inc/ or -I"${workspace_loc:/${ProjName}/inc}"
 * 4.gcc add -fPIC for shared libraries
 * 5.gcc add -lpopt -lpthread -lcrypto -lrt
 */

#include "plcdn_cpp_test.h"		/*test_nginx_log_stats_main*/
#include "test_options.h"	/*nls_options**/
#include "string_util.h"	/*strlwr*/
#include <stdio.h>
#include <string.h> 	/*strncpy*/
#include <unordered_map> 	/*std::unordered_map*/

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
		if(linecount % 5000 == 0)
			fprintf(stdout, "processing %8ld line ...\r", linecount);
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

