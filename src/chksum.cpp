#include "string_util.h"
#include <stdio.h>

#if(!defined _WIN32 && (defined __GNUC__ || defined  __CYGWIN_GCC__))
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
#endif
