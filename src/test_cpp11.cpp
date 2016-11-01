#include "bd_test.h"
/*!
 * test for c++11
 */
static int test_lamda(int argc, char ** argv)
{
	int girls = 3, boys = 4;
	auto total = [](int g, int b){ return g +b; };
	//error
//	auto total_2 = [=](int g, int b){ girls = 5; return g +b; };
	auto sum_func = [](int & sum, int const& val) { return sum += val; };
	auto total_num = total(girls, boys);
	int sum = 3, val = 5;
	auto b = sum_func(sum, val);
	return total_num;
}
int test_cpp11_main(int argc, char ** argv)
{
	test_lamda(argc, argv);
	return 0;
}
