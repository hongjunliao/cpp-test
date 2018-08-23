/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/8/23
 *
 * NAT with iptables
 *
 * libsqlite3-dev, sqlite3
 * */
#include <stdio.h> 	/*  */
#include <stdlib.h> 	/*  */
#include <string.h> 	/*  */
#include <sqlite3.h>

/////////////////////////////////////////////////////////////////////////////////////
int hp_iptables_redirect_port(int port) {
	fprintf(stderr, "%s: NOT implemented yet\n", __FUNCTION__);
	return -1;
}

int hp_nat_run(sqlite3 * db)
{
	int i, j;
	char * p, * errmsg = 0;
	//CREATE TABLE xhhp_tb_service(id INTEGER PRIMARY KEY, name VARCHAR(12), src_addr VARCHAR(12), dst_addr VARCHAR(12), nat INTEGER);

	char buf[1024];
	char const * sql =
			"CREATE TABLE xhhp_tb_service("
			"id INTEGER PRIMARY KEY,"
			"name VARCHAR(12),"
			"src_addr VARCHAR(12),"
			"dst_addr VARCHAR(12),"
			"protocol INTEGER,"
			"nat INTEGER);";
	int rc = sqlite3_exec(db, sql, 0, 0, &errmsg);

	int nrow = 0, ncolumn = 0;
	char ** rs; //results
	sql = "SELECT * FROM xhhp_tb_service;";
	sqlite3_get_table(db, sql, &rs, &nrow, &ncolumn, &errmsg);

	int n = 0;
	char const * natfmt = "iptables -tnat -A PREROUTING -d %s -p %s -m %s --dport %s -j REDIRECT --to-ports %d";
	char * nat_d, * nat_p = "tcp", * nat_dport;
	int nat_to_ports = 9200;
	for (i = 0; i < (nrow + 1) * ncolumn; i += ncolumn){
		for(j = 0; j < ncolumn; ++j)
			printf("%s,", rs[i + j]);
		printf("\n");
		if(i == 0) continue;

		nat_d = rs[i + 2];
		p = strchr(nat_d, ':');
		if(!p) continue;
		nat_dport = p + 1;
		*p = '\0';

		int proto = atoi(rs[i + 4]);
		if(proto == 2) nat_p = "udp";

		n += snprintf(buf + n, sizeof(buf) - n, natfmt, nat_d, nat_p, nat_p, nat_dport, nat_to_ports);
	}
	sqlite3_free_table(rs);

	printf("%s: cmd='%s'\n", __FUNCTION__, buf);
	system(buf);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
int test_nat_main(int argc, char ** argv)
{
	sqlite3 * db;
	int rc = sqlite3_open(argv[1], &db);
	if (rc != 0) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	rc = hp_nat_run(db);
	sqlite3_close(db);
}
char const * help_test_nat() { return "--sqlite3-dbfile=STRING"; }
