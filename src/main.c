/*! This file is PART of clrs project
 * @author hongjun.liao <docici@126.com>, @date 2021/6/12
 *
 * */
#include <assert.h> /* assert */
#include <stdio.h>
/*
 */
int test_max_subarray_main(int argc, char ** argv);
int test_9787302356288_tree_main(int argc, char ** argv);
int test_hp_poll_main(int argc, char ** argv);
int test_algorithms4th_1_5_main(int argc, char ** argv);
int test_algs4th_sort_main(int argc, char ** argv);
int test_greedy_main(int argc, char ** argv);

int main(int argc, char ** argv)
{
	int rc;
	rc = test_greedy_main(argc, argv);assert(rc == 0);
	rc = test_algs4th_sort_main(argc, argv);assert(rc == 0);

	rc = test_algorithms4th_1_5_main(argc, argv);assert(rc == 0);
#if !defined(__WIN32) && !defined(_MSC_VER)
	rc = test_hp_poll_main(argc, argv); assert(rc == 0);
#endif
	rc = test_9787302356288_tree_main(argc, argv); assert(rc == 0);
	rc = test_max_subarray_main(argc, argv); assert(rc == 0);
	return 0;
}
