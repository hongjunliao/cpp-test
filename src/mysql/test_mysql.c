/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/4/20
 *
 * test mysql, currently libmysqlclient
 * TODO: add test for libmysqld-dev
 * github: https://github.com/allidev/mysqlclient
 *
 * (1)ubuntu: apt install libmysqlclient-dev
 * */
#ifdef CPP_TEST_WITH_MYSQL
#include <stdio.h>
#ifndef WITHOUT_LIB_MYSQLCLIENT
#include <string.h> 	/* strlen */
#include <stdio.h>
#include <stdlib.h> 	/* calloc */
#include <string.h>     /* memset, ... */
#include <errno.h>      /* errno */
#include <assert.h>     /* define NDEBUG to disable assertion */
#include "cp_log.h"     /* cp_loge,... */
#include "mysql/mysql.h"     /* ,... */


int test_libmysqlclient_main(int argc, char ** argv)
{
	const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
	int port = (argc > 2) ? atoi(argv[2]) : 3306;
	char const * usr = (argc > 3)? argv[3] : "root";
	char const * passwd = (argc > 4)? argv[4] : "xinghuo@pass";
	char const * db = (argc > 5)? argv[5] : "db_im3_center";
	char * stmt_str = (argc > 6)? argv[6] : "select * from tb_nodetype;";
	if(stmt_str[0] == '\''){
		stmt_str[0] = ' ';
		char * p = strrchr(stmt_str, '\'');
		if(p) *p = '\0';
	}


	MYSQL mysqlobj, * mysql = 0;
	mysql = mysql_init(&mysqlobj);
	if(!mysql){
		cp_loge("mysql_init failed, mysqlerr='%s'\n", 0);
		return -1;
	}

	cp_logd("connecting to mysql %s@%s:%d/%s\n", usr, hostname, port, db);

	mysql = mysql_real_connect(mysql, hostname, usr, passwd, db, port, 0, 0);
	if(!mysql){
		cp_loge("mysql_real_connect failed, mysqlerr='%s'\n",0);
		return -1;
	}


	int length = strlen(stmt_str);

	cp_logd("mysql_real_query, sql='%s'\n", stmt_str);

	int r = mysql_real_query(mysql, stmt_str, length);
	if(r != 0){
		cp_loge("mysql_real_query('%s') failed, mysqlerr='%s'\n", stmt_str, mysql_error(mysql));
		return -1;
	}

	MYSQL_RES * res = mysql_store_result(mysql);
	if(!res){
		int ret = mysql_field_count(mysql);
		if (ret == 0) {
			int ret = mysql_affected_rows(mysql);
			cp_logd("Query OK, %d rows affected\n", ret);
		} else {
			cp_loge("Error: %s\n", mysql_error(mysql));
		}
		return -1;
	}

	printf("----------------------------\n");

	int num_fields = mysql_num_fields(res);
	int num_rows = mysql_num_rows(res);

	MYSQL_FIELD* fields;
	fields = mysql_fetch_fields(res);
	for (int i = 0; i < num_fields; ++i) {
		printf("%20s\t", fields[i].name);
	}
	printf("\n");

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res))) {
		unsigned long *lengths;
		lengths = mysql_fetch_lengths(res);
		for (int i = 0; i < num_fields; ++i) {
			printf("%20s", row[i] ? row[i] : "NULL");
		}
		printf("\n");
	}
	printf("----------------------------\n");
	mysql_free_result(res);


	return 0;
}

#else
int test_libmysqlclient_main(int argc, char ** argv)
{
	cp_loge(stderr, "%s: undefine WITHOUT_LIB_MYSQLCLIENT and install libmysqlclient to enable this test\n"
			, __FUNCTION__);
	return -1;
}
#endif /* WITHOUT_LIB_MYSQLCLIENT */

char const * help_test_libmysqlclient() {
	return "<host> <port> <user> <password> <database> <SQL>\n    "
			"sample: 172.29.3.76 8306 root xinghuo@pass db_im3_center "
			"'select * from tb_nodetype;'";
}
#endif
