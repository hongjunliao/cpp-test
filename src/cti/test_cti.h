/*!
 * 数据结构与算法分析(C++描述)_第３版/Cracking_The_Interview
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * */

#ifndef TEST_CRACKING_THE_CODING_INTERVIEW
#define TEST_CRACKING_THE_CODING_INTERVIEW

#include <string> /* std::string */
#include "boost/pending/disjoint_sets.hpp"	//不相交集
#include "boost/unordered_map.hpp"

//////////////////////////////////////////////////////////////////////////
//汉诺塔问题:将@n个盘子从@a移动到@c, 
//期间要求所有柱子(@a, @b, @c)上的盘子小的在上, 可使用@b 
void Hanoi(char a, char b, char c, int n);
void my_strcpy(char* from, char const* to);
//归并排序, length为数组长度
void merge_sort(int * arr, size_t length);
//归并己排序数组
//满足p << q < r, arr[p..q], arr[q + 1..r]己排好序, p, q, r 为数组下标
void my_merge(int * arr, size_t p, size_t q, size_t r);
//***********************
//利用字符出现的字数, 实现基本的压缩, 如输入"aabcccccaaa", 输出"a2b1c5a3"
void compress_repeat_str(char const* src, size_t length, char* dest);
//若MxN矩阵中某个元素为0, 则将其所在行列清0
void clear_matrix(int** matrix, size_t m, size_t n, int val = 0);
//比较[begin, end)区间字符各不相同
bool all_diff(std::string::const_iterator begin, std::string::const_iterator end);
//反转nulll结尾的字符串
char * my_reverse(char * str);
//比较元素是否相同
bool elements_same(std::string const& a, std::string const& b);
//替换字符为指定字符串
char* replace_char_to_str(char* src, char const& c, char const* str);
//插入排序
void insertion_sort(int * arr, size_t length);
//产生随机数
int roll_die(int start, int end);
//最大子数组
void maximun_subarray(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high);
//最大子数组2,含求low, high
void maximun_subarray2(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high);
//二分搜索, @arr已排好序
int binary_search(int const* arr, int length, int val);
//欧里几得算法求最大公因数
long gcd(long m, long n);
//中缀表达式转后缀表达式
//@expre 中缀表达式, @postfix 后缀表达式
//例 a + b * c + (d * e + f) * g 转 a b c * + d e * f + g * +
void infix2postfix(char const* infix, char* postfix);
//比较运算符优先级(限'+', '-', '*', '/', '('), 
//如果inputOpa > stackOpb 返回true
// '+'  == '-' < '*' == '/'
//特别地对'(', 若处栈中(stackOpb == '(')则优先级最低
bool higher_priority(char inputOpa, char stackOpb);
//计算算术表达式的值, 限四则运算,可含括号
//不含语法检查
double calc4(std::string const& exp);
//Josephus问题
template<typename T> T Josephus(T* child, int n, int m);
//////////////////////////////////////////////////////////////////////////
class maze_cell{
public:
	int row_, col_;
	bool edge_[4];
};
static inline bool operator!=(maze_cell const& lhs, maze_cell const& rhs){
	return lhs.row_ != rhs.row_ && lhs.col_ != rhs.col_;
}

//不相交集应用: 迷宫生成, boost.disjoint_sets实现 □ ■
void maze(int m, int n);
//////////////////////////////////////////////////////////////////////////
//输出数组
std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list);

//快速排序:期望时间复杂度为theta(nlgn)
//划分函数
template<typename T>
int quicksort_partition(T & a, int p, int r);
//快速排序
template<typename T>
void quicksort(T & a, int p, int r);
//std::qsort比较函数
int compare_ints(const void* a, const void* b);

#endif /* TEST_CRACKING_THE_CODING_INTERVIEW */
