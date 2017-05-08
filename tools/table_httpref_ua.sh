#!/bin/bash
# for user/suwang(http://www.fastcache.com.cn/)
# import data to table: acoway_oss_test1.httpref_ua
# @author hongjun.liao <docici@126.com>
#

import_data_to_httpref_ua()
{
	type mysqldbimport || {
		printf "install mysql-utilities for command: mysqldbimport, mysqlimport\n";
		return -1;
	}
	# create table httpref_ua if needed
	mysqldbimport --server=ftpceshi:123@114.119.4.69:3307 /home/jun/plcdn/plcdn-lame-importer.git/tools/table_httpref_ua.sql
	[ $? -ne 0 ] && {
		printf "mysqldbimport failed, exit\n";
		return -1;
	}

	[ -d "/tmp/import/" ] || mkdir -p "/tmp/import/";
	# cat all data to one file 'httpref_ua.txt', as 'httpref_ua' will be the table name
	cat /tmp/output/httprefua* > /tmp/import/httpref_ua.txt
	# now import data to table
	# we use -r(--replace) option because plcdn_la --nginx-rotate-dir option specified, see plcdn_la -h for defails
	mysqlimport -h 114.119.4.69 --port 3307 -u ftpceshi -p123 --local -r --fields-terminated-by=' ' --delete acoway_oss_test1 /tmp/import/httpref_ua.txt

	[ $? -ne 0 ] && {
		printf "mysqlimport failed, exit\n";
		return -1;
	}
	printf "import data to table httpref_ua done\n";
	return 0;
}

# main
import_data_to_httpref_ua;
