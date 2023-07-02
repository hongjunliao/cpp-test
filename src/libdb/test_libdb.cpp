/*!
 * This file is PART of xh_http_proxy project
 * @author hongjun.liao <docici@126.com>
 *
 * https://github.com/berkeleydb/libdb/blob/master/examples/cxx/AccessExample.cpp
 * */
/////////////////////////////////////////////////////////////////////////////////////////////
#include "cpp_test.h"
#include <db_cxx.h>
#include <iostream>
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////////

int test_libdb_main(int argc, char ** argv)
{
	int ret;
	// There is no environment for this simple example.
	char kbuf[] = "key1", vbuf[256] = "key1's value";
	for(;;){
		Db db(0, 0);
		db.open(NULL, "test.db", NULL, DB_BTREE, DB_CREATE, 0);
		Dbt key(kbuf, sizeof(kbuf)), data(vbuf, sizeof(vbuf));
		ret = db.put(0, &key, &data, DB_NOOVERWRITE);
		if (ret == DB_KEYEXIST) {
			// We put a try block around this section of code
			// to ensure that our database is properly closed
			// in the event of an error.
			//
			try {
				// Acquire a cursor for the table.
				Dbc *dbcp;
				db.cursor(NULL, &dbcp, 0);

				// Walk through the table, printing the key/data pairs.
				Dbt key;
				Dbt data;
				while (dbcp->get(&key, &data, DB_NEXT) == 0) {
					char *key_string = (char*) key.get_data();
					char *data_string = (char*) data.get_data();
					cout << key_string << " : " << data_string << "\n";
				}
				dbcp->close();
			} catch (DbException &dbe) {
				cerr << "AccessExample: " << dbe.what() << "\n";
			}
			break;
		}
		db.close(0);
	}
	return 0;
}
