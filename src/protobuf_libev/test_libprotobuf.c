/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/20
 *
 * test libprotobuf
 *
 * (1)ubuntu: apt install libprotobuf-dev protobuf-c-compiler  protobuf-compiler libprotobuf-c-dev

 * (2)compile *.proto:
 * protoc http.proto --cpp_out .
 * or
 * protoc-c http.proto --c_out .
 * */
#include <stdio.h>
#ifndef WITHOUT_LIBPROTOBUF_LIBEV
#include "http.pb-c.h"
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */

int test_libprotobuf_main(int argc, char ** argv)
{
	HttpMsg msg, * msg2  = 0;
	http_msg__init(&msg);
	msg.url = "http://127.0.0.1/index.html";

	size_t N = http_msg__get_packed_size(&msg);
	uint8_t * out = malloc(N * 2);
	size_t r = http_msg__pack(&msg, (uint8_t *)out);
	if(r != N){
		fprintf(stdout, "%s: pack failed, size=%zu, pack_size=%zu\n", __FUNCTION__, N, r);
		free(out);
		return -1;
	}

	char const * outfile = "/tmp/test_libprotobuf_libev_main";
	FILE * f = fopen(outfile, "w");
	assert(f);
	r = fwrite(out, sizeof(char), N, f);
	if(r != N){
		fprintf(stderr, "%s: fwrite failed, size=%zu, return=%zu, errno=%d, error='%s'\n"
				, __FUNCTION__
				, N, r
				, errno, strerror(errno));
		free(out);
		return -1;
	}
	fprintf(stdout, "%s: msg_pack, msg=%p, url='%s'\n", __FUNCTION__
			, &msg, msg.url);


	memset(out, 0, N * 2);
	f = freopen(outfile, "r", f);
	assert(f);

	r = fread(out, sizeof(char), N, f);
	if(r != N){
		fprintf(stderr, "%s: fread failed, size=%zu, return=%zu, errno=%d, error='%s'\n"
				, __FUNCTION__
				, N, r
				, errno, strerror(errno));
		free(out);
		return -1;
	}
	fclose(f);

	msg2 = http_msg__unpack(0, N, out);
	fprintf(stdout, "%s: msg_unpack, msg=%p, url='%s'\n", __FUNCTION__
			, msg2, msg2? msg2->url : 0);

	assert(msg2 && strcmp(msg.url, msg2->url) == 0);

	http_msg__free_unpacked(msg2, 0);

	free(out);
	return 0;
}

#else
int test_libprotobuf_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: undefine WITHOUT_LIBPROTOBUF_LIBEV and install libprotobuf to enable this test\n"
			, __FUNCTION__);
	return -1;
}
#endif /* WITHOUT_LIBPROTOBUF_LIBEV */


char const * help_test_libprotobuf()
{
	return "libprotobuf_libev";
}
