#include "bd_test.h"
#include <stdio.h>
#include <openssl/md5.h> /*openssl MD5()*/

char const * md5sum(char const * str, int len)
{
	unsigned char md[16];
	static char ret[33];
	MD5((unsigned char const *)str, len, md);
	snprintf(ret, 33, "%02x%02x%02x%02x%02x%02x%02x%02x"
			"%02x%02x%02x%02x%02x%02x%02x%02x",
			md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7],
			md[8], md[9], md[10], md[11], md[12], md[13], md[14], md[15]
	);
	ret[32] = '\0';
//	fprintf(stdout, "str=%-50s, len=%-10d, md5sum=%-35s\n", str, len, ret);
	return ret;
}

#if (defined __GNUC__) && !(defined __CYGWIN__)

char *strlwr(char *s)
{
	for (char *str = s; *str; ++str) {
		if (*str >= 'A' && *str <= 'Z') {
			*str += ('a' - 'A');
		}
	}
	return s;
}

char *strupr(char *s)
{
	for (char *str = s; *str; ++str) {
		if (*str >= 'a' && *str <= 'z') {
			*str -= ('a' - 'A');
		}
	}
	return s;
}

#endif	/*(defined __GNUC__) && !(defined __CYGWIN__)*/

/*@param unit 1: KB, 2:MB, 3:GB, 0:B*/
double byte_to_mb_kb(size_t bytes, char & unit)
{
	if(bytes >= 1024 * 1024 && bytes < 1024 * 1024 * 1024){
		unit = 'M';
		return bytes / (1024.0 * 1024);
	}
	else if(bytes >= 1024 * 1024 * 1024){
		unit = 'G';
		return bytes / (1024.0 * 1024 * 1024);
	}
	else if(bytes >= 1024){
		unit = 'K';
		return bytes / 1024.0;
	}
	else{
		unit = ' ';
		return bytes / 1.0;
	}
}

/*@param fmt: "%-.2f %cB*/
char const * byte_to_mb_kb_str(size_t bytes, char const * fmt)
{
	static char buff[64] = "";
	if(!fmt) return buff;
	char c;
	double b = byte_to_mb_kb(bytes, c);
	snprintf(buff, 64, fmt, b, c);
	return buff;
}
