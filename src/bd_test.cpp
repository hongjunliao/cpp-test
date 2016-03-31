#include "bd_test.h"
#include <algorithm>
static int const BD_TEST_SZ = 512;
static test_main_fn test_main_fns[BD_TEST_SZ];
static int test_main_fn_curr_index = 0;

void test_main_add(test_main_fn fn)
{
	bool is_nfound = (std::end(test_main_fns) == std::find(std::begin(test_main_fns), std::end(test_main_fns), fn));
	if(is_nfound)
		test_main_fns[test_main_fn_curr_index++] = fn;
}
