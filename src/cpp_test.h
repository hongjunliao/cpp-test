/*!
 * include file for cpp-test
 * @author hongjun.liao<docici@126.com>
 */

#ifndef CPP_TEST_H_
#define CPP_TEST_LOG_H_
#ifdef __cplusplus
extern "C"{
#endif

struct test_entry{
	char const * name;
	int(*main)(int argc, char ** argv);
	char const * (* help)();
};

void register_test(struct test_entry const * tst);
/**
 * xlib util
 */
int xlib_draw_rectangle(int x, int y, int w, int h);

/*mykeys.cpp*/


/*!
 * @param sep, seperator: "\r\n"
 */
int mykeys_regex_search(char const * key_file, char const * password, char const * keyword,
		char const * sep, char * matches, int len);
/*error code*/
#define BD_TEST_NO_SUCH_TEST -2

/*call test, no @param stest then return @see BD_TEST_NO_SUCH_TEST*/
int bd_test_main(int argc, char ** argv, char const * stest);
char const * bd_test_get_test_list();

int test_algs4th_sort_main(int argc, char ** argv);

int test_boost_asio_main(int argc, char **argv);
int test_max_subarray_main(int argc, char ** argv);
int test_9787302356288_tree_main(int argc, char ** argv);
int test_hp_poll_main(int argc, char ** argv);
int test_algorithms4th_1_5_main(int argc, char ** argv);
int test_algs4th_sort_main(int argc, char ** argv);
int test_greedy_main(int argc, char ** argv);
int test_dbus_main(int argc, char **argv);
int test_libwx_main(int argc, char ** argv);
int test_uv_main(int argc, char ** argv);
int test_openssl_main(int argc, char ** argv);
#ifdef __cplusplus
}
#endif	//__cplusplus
#endif /* CPP_TEST_H_ */
