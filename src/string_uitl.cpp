#include "string_util.h"
#include <stdio.h>

#if (defined __GNUC__ || defined  __CYGWIN_GCC__)
/*coreutils-8.21 also ok*/
#include <openssl/md5.h> /*MD5()*/
#include <openssl/sha.h> /*SHA1()*/


static auto HEXTAB="0123456789abcdef";

char * md5sum_r(char const * str, int len, char * buff)
{
	unsigned char md[MD5_DIGEST_LENGTH];
	MD5((unsigned char const *)str, len, md);
	snprintf(buff, 33, "%02x%02x%02x%02x%02x%02x%02x%02x"
			"%02x%02x%02x%02x%02x%02x%02x%02x",
			md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7],
			md[8], md[9], md[10], md[11], md[12], md[13], md[14], md[15]
	);
	buff[32] = '\0';
//	fprintf(stdout, "str=%-50s, len=%-10d, md5sum=%-35s\n", str, len, ret);
	return buff;
}

char * sha1sum_r(char const * str, int len, char * buff)
{
    unsigned char md[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)str, len, md);

    int n = 0;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
    	buff[n++] = HEXTAB[(md[i] >> 4 ) & 0x0F];
    	buff[n++] = HEXTAB[md[i] & 0x0F];
    }
	buff[n] = '\0';
	return buff;
}
#endif /* (defined __GNUC__ || defined  __CYGWIN_GCC__) */

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

/*@param unit 'K': KB, 'M':MB, 'G':GB, ' ':B*/
static double byte_to_mb_kb(size_t bytes, char & unit)
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

char * byte_to_mb_kb_str_r(size_t bytes, char const * fmt, char * buff)
{
	if(!fmt) return buff;
	char c;
	double b = byte_to_mb_kb(bytes, c);
	snprintf(buff, 64, fmt, b, c);
	return buff;
}
/*@param fmt: "%-.2f %cB*/
char const * byte_to_mb_kb_str(size_t bytes, char const * fmt)
{
	static char buff[64] = "";
	return byte_to_mb_kb_str_r(bytes, fmt, buff);
}
