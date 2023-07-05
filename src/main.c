/*! This file is PART of clrs project
 * @author hongjun.liao <docici@126.com>, @date 2021/6/12
 *
 * */
#include "config.h"
#include <assert.h> /* assert */
#include <stdio.h>
#include "cpp_test.h"
#include "hp/string_util.h"
#include "hp/libhp.h"

int do_main(int argc, char ** argv)
{
	int rc;
	rc = libhp_all_tests_main(argc, argv);
//	rc = test_libdb_main(argc, argv);assert(rc == 0);
#ifndef NDEBUG
//	rc = test_hp_str_main(argc, argv);assert(rc == 0);
#endif /* NDEBUG */
#ifdef CPP_TEST_WITH_OPENSSL
	rc = test_openssl_main(argc, argv);assert(rc == 0);
#endif //
#ifdef CPP_TEST_WITH_LIBUV
	rc = test_uv_main(argc, argv);assert(rc == 0);
#endif //WITH_LIBUV
#ifdef CPP_TEST_WITH_DBUS
	rc = test_dbus_main(argc, argv);assert(rc == 0);
#endif //WITH_DBUS

#ifdef CPP_TEST_WITH_BOOST
	rc = test_boost_asio_main(argc, argv);assert(rc == 0);
#endif
	rc = test_greedy_main(argc, argv);assert(rc == 0);
	rc = test_algs4th_sort_main(argc, argv);assert(rc == 0);

	rc = test_algorithms4th_1_5_main(argc, argv);assert(rc == 0);
//	rc = test_hp_poll_main(argc, argv); assert(rc == 0);
	rc = test_9787302356288_tree_main(argc, argv); assert(rc == 0);
	rc = test_max_subarray_main(argc, argv); assert(rc == 0);
	return 0;
}

int main(int argc, char ** argv)
{
	return do_main(argc, argv);
}
