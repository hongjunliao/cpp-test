/*!
 * mykeys, get keys or add keys use openssl enc -d -aes-256-cbc
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/ssl.h>

static char * pwd_file = "docici.acc";	/*passord file*/
static char * keyword = 0;

static void show_usage()
{
	fprintf(stdout, "cmd password_file(default docici.acc) key_word_to_search\n");
}
int mykeys_main(int argc, char ** argv)
{
	if(argc <= 1){
		show_usage();
		return -1;
	}
	if(argc > 1 && argc <= 2){
		if(access(pwd_file, 0) == -1){
			show_usage();
			return -1;
		}
	}
	pwd_file = argv[1];
	if(access(pwd_file, 04) == -1){
		fprintf(stderr, "can NOT read file:%s\n", pwd_file);
		return -1;
	}

	return 0;
}
