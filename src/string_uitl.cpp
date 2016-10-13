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
