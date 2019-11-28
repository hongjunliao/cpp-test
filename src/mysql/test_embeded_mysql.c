/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2019/11/28
 *
 * */
#include <stdio.h>
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include "mysql/mysql.h"     /* ,... */
#include "sds/sds.h"

int test_embeded_mysql_main(int argc, char ** argv)
{
	int rc;
	static char *server_options[] = { "mysql_test", // An unused string
			"--datadir=/tmp/mysql_embedded_data", // Your data dir
			NULL };
	int num_elements = (sizeof(server_options) / sizeof(char *)) - 1;

	static char *server_groups[] = { "libmysqld_server", "libmysqld_client",
			NULL };

	// Init MySQL lib and connection
	mysql_library_init(num_elements, server_options, server_groups);

	rc = mysql_thread_init();

	MYSQL *con = mysql_init(NULL);

	if (con == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		exit(1);
	}

	mysql_options(con, MYSQL_READ_DEFAULT_GROUP, "libmysqld_client");
	mysql_options(con, MYSQL_OPT_USE_EMBEDDED_CONNECTION, NULL);

	// Connect to no host/port -> Embedded mode
	if (mysql_real_connect(con, NULL, NULL, NULL, 0, 0, NULL, 0) == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		exit(1);
	}

	// Create a sample empty DB, named "aNewDatabase"
//	if (mysql_query(con, "CREATE DATABASE aNewDatabase")) {
//		fprintf(stderr, "%s\n", mysql_error(con));
//		mysql_close(con);
//		exit(1);
//	}


	sds sql = sdsempty();
	char bufline[2048];
	int line = 1;
	for(; fgets(bufline, sizeof(bufline), stdin); ++line){
		sql = sdscat(sql, bufline);
	}

	rc = mysql_query(con, sql);
	if(rc != 0){
		fprintf(stderr, "%s: mysql_query('%s') failed, err='%s'\n", __FUNCTION__, sql, mysql_error(con));
	}


	sdsfree(sql);

	// Close connection
	mysql_close(con);

	mysql_thread_end();
	mysql_library_end();

	fprintf(stdout, "%s: done\n", __FUNCTION__);

	return 0;
}
