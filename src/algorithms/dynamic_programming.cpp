/*!
 * test dynamic_programing/动态规划 from <Introduction_to_Algorithms 3th Edition>_15.2
 * @author hongjun.liao <docici@126.com>, @date 2017//05/08
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <iostream>  /* std::std::cout, ... */
#include <limits>    /* std::numeric_limits */
#include <vector>    /* std::vector */
#include <iterator>  /* std::ostream_iterator */
#include <limits.h>  /* INT_MIN */
#include <map>       /* std::map */

//钢条切割问题:
//给定钢条长度与价格的关系表p, 求长度为N的钢条如何切割获得的收益最大
//@param p, 价格数组
//@param n 长度
//@param(out) it 切割方案
//自顶向下的递归实现
static int cut_rod(int const* p, int n);

//带备忘的自底向上实现
int bottom_up_cut_rod(int const* p, int n);

template<typename InsertIerator>
int bottom_up_cut_rod(int const* p, int n, InsertIerator it);

//////////////////////////////////////////////////////////////////////////
static int cut_rod(int const* p, int n){
	if( 0 == n)
		return 0;
	int q = INT_MIN;
	for(int i = 1; i <= n; ++i)
		q = std::max(p[i - 1], cut_rod(p, n - i));
	return q;
}
int bottom_up_cut_rod(int const* p, int n){
	std::map<int, int> r;	//长度与最大收益映射
	r[0] = 0;
	for(int j = 1; j <= n; ++j){
		int q = std::numeric_limits<int>::max();
		for(int i = 1; i <= j; ++i)
			q = std::max(q, p[i - 1] + r[j - i]);
		r[j] = q;
	}
	return r[n];
}
template<typename InsertIerator>
int bottom_up_cut_rod(int const* p, int n, InsertIerator it){
	std::map<int, int> r, s;	////长度与最大收益, 第一段钢条的切割长度映射,
	r[0] = 0;
	for(int j = 1; j <= n; ++j){
		int q = INT_MIN;
		for(int i = 1; i <= j; ++i)
			if(q < p[i - 1] + r[j - i]){
				q = p[i - 1] + r[j - i];
				s[j] = i;
			}
		r[j] = q;
	}
	for(std::map<int, int>::const_iterator iter = s.begin(); iter != s.end(); ++iter)
		*it = iter->second;
	return r[n];
}

int test_cut_rod_main(int argc, char ** argv)
{
	int prices[] = {1, 5, 8, 9, 10, 17, 17, 20, 24, 30};
	int const length = sizeof(prices) / sizeof(prices[0]);
	std::vector<int> solutions;
	std::cout << "动态规划:钢条切割问题";
	std::copy(prices, prices + length, std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl << "最大收益(自顶向下递归实现):" << cut_rod(prices, length) << std::endl;
	std::cout << std::endl << "最大收益(自底向上):" << bottom_up_cut_rod(prices, length) << std::endl;
	std::cout << std::endl << "最大收益(自底向上, 并返回输出切割方案):"
		<< bottom_up_cut_rod(prices, length, std::back_inserter(solutions)) << std::endl;
	for(size_t n = solutions.size(); n > 0; n = n - solutions[n - 1])
		std::cout << solutions[n - 1] << ", ";
	std::cout << std::endl;

	return 0;
}

int test_dynamic_programming_main(int argc, char ** argv)
{
	//动态规划:钢条切割问题
	auto ret = test_cut_rod_main(argc, argv);

	return ret;
}
