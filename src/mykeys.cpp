#ifdef CPP_TEST_WITHMYKEYS

#ifdef __CYGWIN__
/*!
 * mykeys, get keys or add keys use openssl enc -d -aes-256-cbc
 */
#include "plcdn_cpp_test.h"
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <regex>
#include <regex>

#include "termio_util.h"	/*set_disp_mode*/


static std::vector<std::string> keys;		/*all keys, in memory*/
static bool is_debug = false;
#define COUNT_OF(arr) (sizeof(arr) / sizeof(arr[0]))

static void openssl_stdout(void * data, int length);
static int mykeys_regex_search(char const * key_file, char const * password, char const * keyword, std::vector<std::string>& matches);
/*test_win32_pipe.cpp*/
extern int execute_with_wait_pipe(char * cmd, void (*stdout_cb)(void * data, int length));

static void show_usage(char const * argv0)
{
	fprintf(stdout, "%s key_file(default docici.acc) keyword (password | --debug | -d)\n", argv0);
}

int mykeys_main(int argc, char ** argv)
{
	for(int i = 0; i < argc; ++i){
		fprintf(stdout, "argv[%02d]:\t%s\n", i, argv[i]);
	}
	if(argc <= 1){
		show_usage(argv[0]);
		return -1;
	}
	static char const * key_file = "docici.acc",	/*key file*/
			* keyword = "",			/*key word to search*/
			* password = "";		/*password*/
	int pwd_index = 2;
	if(argc == 2){
		keyword = argv[1];
	}
	else{
		key_file = argv[1];
		keyword = argv[2];
		pwd_index = 3;
	}
	if(strcmp(argv[pwd_index], "-d") == 0 || strcmp(argv[pwd_index], "--debug") == 0)
		is_debug = true;
	else
		password = argv[pwd_index];

	if(is_debug){
		fprintf(stdout, "--debug set, input password:");
		fflush(stdout);
		termio_set_disp_mode(STDIN_FILENO, 0);
		static char szpwd[64] = "\n";
		while(szpwd[0] == '\n'){
			if(!fgets(szpwd, COUNT_OF(szpwd), stdin)){
				fprintf(stderr, "get password failed\n");
				return -1;
			}
		}
		fprintf(stdout, "\n");
		char * pchr = strrchr(szpwd, '\n');
		if(pchr) *pchr = '\0';

		password = szpwd;
		termio_set_disp_mode(STDIN_FILENO, 1);
	}
	if(!(strlen(key_file) > 0 && strlen(keyword) > 0 && strlen(password) > 0)){
		show_usage(argv[0]);
		return -1;
	}

	std::vector<std::string> matches;
	if(mykeys_regex_search(key_file, password, keyword, matches) != 0){
		return -1;
	}
	for(auto iter  = matches.begin(); iter != matches.end(); ++iter){
		fprintf(stdout, "%s\n", iter->c_str());
	}
	return 0;
}

static int mykeys_regex_search(char const * key_file, char const * password, char const * keyword, std::vector<std::string>& matches)
{
	char cmd[128] = "";
	snprintf(cmd, COUNT_OF(cmd), "openssl enc -d -aes-256-cbc -in %s -pass pass:%s",
			key_file, password? (is_debug? "***" : password) : "<null>");
	if(is_debug){
		fprintf(stdout, "%s: cmd %s\n", __FUNCTION__, cmd);
	}
	keys.clear();
	if(execute_with_wait_pipe(cmd, openssl_stdout) != 0){
		fprintf(stderr, "decript failed\n");
		return -1;
	}
	if(keys.empty()){
		fprintf(stderr, "key_file empty\n");
		return 0;
	}
	std::regex reg(keyword, std::regex::icase);
	std::smatch sm;
	for(auto iter  = keys.begin(); iter != keys.end(); ++iter){
		if(regex_search(*iter, sm, reg))
			matches.push_back(iter->c_str());
	}
	return 0;
}

int mykeys_regex_search(char const * key_file, char const * password, char const * keyword,
		char const * sep, char * matches, int len)
{
	std::vector<std::string> smatches;
	int ret = mykeys_regex_search(key_file, password, keyword, smatches);
	if(ret != 0){
		return ret;
	}
	std::string s;
	for(auto const& item : smatches){
		s += item;
		s += sep? sep : "";
	}
	strncpy(matches, s.c_str(), len);
	return ret;
}

static void openssl_stdout(void * data, int length)
{
	char * cdata = (char * )data;
	for(char * p = cdata, *q = cdata; *q; ++q){
		if(*q == '\n'){
			std::string line(p, q - p);
			keys.push_back(line);
			p = ++q;
//			if(is_debug)
//				fprintf(stdout, "%s\n", line.c_str());
		}
	}
}

#endif /*__CYGWIN__*/

#endif

