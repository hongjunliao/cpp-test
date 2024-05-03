/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>
 * */
#include "config.h"
#include "hp/string_util.h"
#include <stdio.h>

#ifdef CPP_TEST_WITH_OPENSSL
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
//	fprintf(stdout, "str=%s, len=%d, md5sum=%-35s\n", str, len, buff);
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

char * md5sum_file_r(char const * f, char * buff)
{
	unsigned char c[MD5_DIGEST_LENGTH];

	FILE *inFile = fopen(f, "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (!inFile) {
		return 0;
	}

	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (c,&mdContext);

	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(buff + i * 2, "%02x", c[i]);
	}

	fclose (inFile);

	return buff;
}
#endif /* CPP_TEST_WITH_OPENSSL*/
