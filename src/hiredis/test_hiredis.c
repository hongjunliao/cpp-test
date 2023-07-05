/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/20
 *
 * test libhiredis
 * sample command:
 * cpp-test-main hiredis 172.29.3.76 7001
 *
 * (1)ubuntu: apt install libhiredis-dev for github: https://github.com/redis/hiredis
 * */
#ifdef CPP_TEST_WITH_HIREDIS
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include "hiredis/hiredis.h"
#include "cp_log.h"     /* cp_loge,... */

int test_libhiredis_main(int argc, char ** argv)
{
	redisContext *c;
	redisReply *reply;
	const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
	int port = (argc > 2) ? atoi(argv[2]) : 6379;

	cp_logd("connecting to '%s:%d'\n", hostname, port);

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (!c|| c->err) {
		if (c) {
			cp_loge("Connection error: %s\n", c->errstr);
			redisFree(c);
		} else {
			cp_loge("Connection error: can't allocate redis context\n");
		}
		exit(1);
	}

	/* PING server */
	reply = redisCommand(c, "PING");
	cp_logd("PING: %s\n", reply->str);
	freeReplyObject(reply);
}

char const * help_test_libhiredis() {
	return "<host> <port>\n    sample: 172.29.3.76 7001";
}
#endif
