//============================================================================
// Name        : test_main.cpp
// Author      : liaohj
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <stdio.h>
#include <string>
#include "bd_test.h"

static int show_help(int, char **);

int main(int argc, char *argv[]) {
	int ret = 0;
	std::string stest = (argc > 1 ? argv[1] : "");
	if(stest.empty() || stest == "help"){
		show_help(argc, argv);
		return 0;
	}

	static int act_argc = 0;
	static char *act_argv[256] = {0};
	for(int i = 0; i < argc; ++i){
		if(i == 1) continue;
		act_argv[act_argc] = argv[i];
		++act_argc;
	}

	ret = bd_test_main(act_argc, act_argv, stest.c_str());
	return ret;
}

static int show_help(int argc, char ** argv)
{
	printf("%s %s\n", argv[0], bd_test_get_test_list());
	return 0;
}

