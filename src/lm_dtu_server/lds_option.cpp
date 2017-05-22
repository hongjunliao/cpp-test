/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * cmdline options
 */
#include "lds_inc.h"
#include <stdio.h>	/* */
#include <stdlib.h>	/* strtol */
#include <string.h>	/* strlen */

lds_options opt = { 8010, "exec sp_insert_wpp $M,$S,$R,$T,$Y,$Z,$A,$B,$C,$D,$E,$F,$G\ngo\n", 0, 0 };

#define is_param(str, len, opt) ((str) && (len) > 2 && strncmp((str), (opt), 2) == 0)
#define is_opt(str, opt)        ((str) && strcmp((str), (opt)) == 0)


int lds_parse_cmdline(int argc, char ** argv)
{
	for(int i = 0; i < argc; ++i){
		size_t len = strlen(argv[i]);
		if(is_param(argv[i], len, "-p")){
			char * s = argv[i] + 2, * end;
			int port = strtol(s, &end, 10);
			if(port <= 0)
				return -1;

			opt.port = port;
		}
		else if(is_param(argv[i], len, "-s"))
			opt.sav = argv[i] + 2;
		else if(is_opt(argv[i], "-h"))
			opt.help = 1;
		else if(is_opt(argv[i], "-v"))
			opt.ver = 1;

	}
	if(opt.port <= 0)
		return -1;
	return 0;
}

void lds_show_usage(char const * cmd, FILE * f)
{
	fprintf(f, "%s, build at %s %s\n", cmd, __DATE__, __TIME__);
	fprintf(f, "usage: %s [OPTIONS]\n", cmd);
	fprintf(f, "    -p<port>        port to listen, sample -p8010\n");
	fprintf(f, "    -h              print this help\n");
	fprintf(f, "    -v              print version info\n");
}

void lds_show_verison(char const * cmd, FILE * f)
{
	fprintf(f, "%s, build at %s %s. Copyright (c) http://www.landmarksz.com\n",
			cmd, __DATE__, __TIME__);
	fprintf(f, "authors:\n    hongjun.liao <docici@126.com>\n");
}
