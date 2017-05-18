/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * */

#if 1
#include "test_cti.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>            /* std::cout, cin, std::endl */
#include <stack>               /* std::stack */
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>  /* boost::split */
#include <boost/foreach.hpp>   /* BOOST_FOREACH */
//////////////////////////////////////////////////////////////////////////
void clear_matrix(int** matrix, size_t m, size_t n, int val){
	for(size_t i = 0; i != m; ++i){
		for(size_t j = 0; j != n; ++j){
			if(matrix[i][j] == val){

			}
		}
	}
}
void Hanoi(char a, char b, char c, int n){
	if(1 == n)	//a上只有一个盘子了
		std::cout << n << ": " << a << " -> " << c << std::endl;
	else{
		Hanoi(a, c, b, n - 1);	//将a上n-1个盘子移动到b
		std::cout << n << ": " << a << " -> " << c << std::endl;
		Hanoi(b, a, c, n - 1);	
	}
}
void my_strcpy(char* from, char const* to){
	while(*to)
		*from++ = *to++;
	*from = 0;
}
void merge_sort(int * arr, size_t length){
	if(!arr || length < 2)
		return;
	size_t q = length / 2;
	merge_sort(arr, q);		//注意第二个参数为数组长度!
	merge_sort(arr + q, length - q);
	my_merge(arr, 0, q - 1, length - 1);
}

void my_merge(int * arr, size_t p, size_t q, size_t r)
{
	size_t const n1 = q - p + 1, n2 = r - q;
	int *L = new int[n1 + 1], *R = new int[n2 + 1];
	std::copy(arr, arr + n1, L);
	std::copy(arr + n1, arr + n1 + n2, R);

	L[n1] = INT_MAX, R[n2] = INT_MAX;	//哨兵

	size_t i = 0, j = 0;
	for(size_t k = p; k <= r; ++k){
		if(L[i] <= R[j]){
			arr[k] = L[i];
			++i;
		}
		else{
			arr[k] = R[j];
			++j;
		}
	}
	delete[] L;
	delete[] R;
}


void compress_repeat_str(char const* src, size_t length, char* dest){
	if(!src || 0 == length)
		return ;
	char const* beg = src, *end = src + length;
	size_t index = 0;
	char c = *beg++;
	size_t count = 1;
	for(; beg != end; ++beg){
		if(c == *beg)
			++ count;
		else{
			index += sprintf(dest + index, "%c%zu", c, count);;
			c = *beg;
			count = 1;
		}
	}
	index += sprintf(dest + index, "%c%zu", c, count);;
	if(index >= length)
		memcpy(dest, src, length);
}
bool all_diff(std::string::const_iterator begin, std::string::const_iterator end)
{
	if(begin == end)
		return true;
	char c = *begin;
	++begin;
	for(std::string::const_iterator iter = begin; iter != end; ++iter){
		if(*iter == c)
			return false;
	}
	return all_diff(begin, end);
}
char * my_reverse(char * str)
{
	size_t length = strlen(str), half = length / 2;
	for(size_t i = 0; i < half; ++i){
		if(i == length - i)
			return str;
		auto c = str[i];
		str[i] = str[length - i - 1];
		str[length - i -  1] = c;
	}
	return str;
} 
bool elements_same(std::string const& a, std::string const& b){
	std::map<char, size_t> charsa, charsb;
	for(std::string::const_iterator iter = a.begin(); iter != a.end(); ++iter)
		++charsa[*iter];
	for(std::string::const_iterator iter = b.begin(); iter != b.end(); ++iter)
		++charsb[*iter];
	return charsa == charsb;
}
char* replace_char_to_str(char* src, char const& c, char const* str){
	std::string const s = src;
	size_t length = strlen(str);
	for(size_t i = 0; i != s.size(); ++i){
		if(s[i] == c){
			strcpy(src, str);
			src += length;
		}
		src[i] = s[i];
	}
	return src;
}

void insertion_sort(int * arr, size_t length){
	if(length < 2 || !arr)
		return;
	for(size_t j = 1; j != length; ++j){
		int i = j - 1, key = arr[j];	//这里i有可能值为负数, arr[j]必须被暂存下来
		for(; i >= 0 && arr[i] > key; --i)
			arr[i + 1] = arr[i];
		arr[i + 1] = key;
	}
}
int roll_die(int start, int end){
	static boost::random::mt19937 gen(std::time(NULL));
	boost::random::uniform_int_distribution<> dist(start, end);
	return dist(gen);
}
void maximun_subarray(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high){
	if(!arr || 0 == length) return;
	int b = 0;
	maxSum = 0;
	for(size_t i = 0; i != length; ++i){
		if(b < 0)
			b = arr[i];
		else 
			b += arr[i];
		if(maxSum < b)
			maxSum = b;
	}
	return;
}
void maximun_subarray2(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high){
	if(!arr || 0 == length) 
		return;
	maxSum =  low = high = 0;
	int thisSum = 0;
	bool lastInc = false;
	for(size_t i = 0; i != length; ++i){
		thisSum += arr[i];
		lastInc = (thisSum > maxSum);
		if(lastInc){
			high = i;
			maxSum = thisSum;
		}
		else{
			if(thisSum < 0)
				thisSum = 0;
		}
	}
	thisSum = maxSum; 
	int i = high;
	for(; thisSum != 0; --i)
		thisSum -= arr[i];
	low = i + 1;
	return;
}

int binary_search(int const* arr, int length, int val){
	if(!arr || 0 == length) 
		return -2;
	int low = 0, high = length;
	while(low <= high){
		int mid = (low + high) / 2;
		if(arr[mid] < val)
			low = mid + 1;
		else if(arr[mid] > val)
			high = mid -1;
		else
			return mid;
	}
	return -1;
}
long gcd(long m, long n){
	if(n > m)
		std::swap(m, n);
	while(n != 0){
		long rem = m % n;
		m = n;
		n = rem;
	}
	return m;
}
double calc4(std::string const& expr)
{
	std::string exp(expr);
	exp.erase(std::remove(exp.begin(), exp.end(), ' '), exp.end()); // Erase ' '
	if(exp.empty())
		return 0.0;
	std::string postfix;
	std::stack<char> operators;
	char top, c;
	for(std::string::const_iterator iter = exp.begin(); iter != exp.end(); ++iter){
		c = *iter;
		switch(c){
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
			while(!operators.empty() && !higher_priority(c, (top = operators.top()))){
				operators.pop();
				postfix += top;
				postfix += ' ';
			}
			operators.push(c);
			break;
		case ')':{
			while((top = operators.top()) != '('){
				operators.pop();
				postfix += top;
				postfix += ' ';
			}
			operators.pop();
			break;
				 }
		default:
			postfix +=  c;
			postfix += ' ';
			break;
		}
	}
	while(!operators.empty()){
		top = operators.top();
		operators.pop();
		postfix += top;
		postfix += ' ';
	}

	std::vector<std::string> tokens;
	boost::split(tokens, postfix, boost::is_any_of(" "));
	std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<std::string>(std::cout, ", "));
	std::cout << std::endl;

	std::stack<std::string> tokenStack;
	for(std::vector<std::string>::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
	{
		if(iter->empty())
			continue;
		char c = iter->at(0);
		if(iter->size() == 1 && (c == '+' || c == '*' || c == '-' || c == '/')){
			double operand1 = std::strtod(tokenStack.top().c_str(), NULL);
			tokenStack.pop();
			double operand2 = std::strtod(tokenStack.top().c_str(), NULL);
			tokenStack.pop();
			char op = c;
			double result = 0.0;
			if('+' == op)
				result = operand1 + operand2;
			else if('-' == op)
				result = operand1 - operand2;
			else if('*' == op)
				result = operand1 * operand2;
			else if('/' == op)
				result = operand1 / operand2;

			char tmp[128];
			std::sprintf(tmp, "%f", result);
			tokenStack.push(tmp);

		}
		else
			tokenStack.push(*iter);
	}
	return std::strtod(tokenStack.top().c_str(), NULL);
}
void infix2postfix(char const* infix, char* postfix)
{
	if(!infix)
		return;
	std::stack<char> operators;
	char top;
	while(*infix){
		switch(*infix){
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
			while(!operators.empty() && !higher_priority(*infix, (top = operators.top()))){
				operators.pop();
				*postfix++ = top;
				*postfix++ = ' ';
			}
			operators.push(*infix);
			break;
		case ')':{
			while((top = operators.top()) != '('){
				operators.pop();
				*postfix++ = top;
				*postfix++ = ' ';
			}
			operators.pop();
			break;
				 }
		default:
			*postfix++ = *infix;
			*postfix++ = ' ';
			break;
		}
		++infix;
	}
	while(!operators.empty()){
		top = operators.top();
		operators.pop();
		*postfix++ = top;
		*postfix++ = ' ';
	}
}
bool higher_priority(char inputOpa, char stackOpb){
	return (inputOpa == '(' || 
		stackOpb == '(' || 
		((inputOpa == '*' || inputOpa == '/') && (stackOpb == '+' || stackOpb == '-'))
		);
	//switch(inputOpa){
	//		case '+':
	//		case '-':
	//			if(stackOpb == '+' || stackOpb == '-')
	//				return false;
	//			else if(stackOpb == '*' || stackOpb == '/')
	//				return false;
	//			else if(stackOpb == '(')
	//				return true;
	//		case '*':
	//		case '/':
	//			if(stackOpb == '+' || stackOpb == '-')
	//				return true;
	//			else if(stackOpb == '*' || stackOpb == '/')
	//				return false;
	//			else if(stackOpb == '(')
	//				return true;
	//		case '(':
	//			return true;
	//}
}
std::ostream& operator<<(std::ostream& ostr, const boost::container::list<int>& list)
{
	fprintf(stdout, "%s: TODO: implement me\n", __FUNCTION__);
//	BOOST_FOREACH(int const& i, list) {
////		ostr << " " << i;
//	}
//	ostr << std::endl;
	return ostr;
}
template<typename T> T Josephus(T* child, int n, int m)
{
	int kickOut = n - 1, curr = -1;	//要剔除的数量, 当前元素下标, 注意下标与数量的区别!
	while(true){
		for(int i = 0; i != m;){	//数m个数,数到下一个元素
			curr  = (curr + 1) % n; //curr在[0, n)之间循环(下标与数量之间的关系运算存在1的差)
			if(child[curr] != INT_MIN)
				++i;
		}
		if(kickOut == 1)
			break;
		std::cout << child[curr] << ", ";
		child[curr] = INT_MIN;		//标记为己剔出
		--kickOut;
	}
	std::cout << std::endl;
	return child[curr];
}
//////////////////////////////////////////////////////////////////////////
void maze(int m, int n){
	using namespace boost;
	std::vector<int>  rank (100);
	std::vector<int>  parent (100);
	boost::disjoint_sets<int*,int*> ds(&rank[0], &parent[0]);
	ds.union_set(0, 2);
}



//////////////////////////////////////////////////////////////////////////
int cut_rod(int const* p, int n){
	if( 0 == n)
		return 0;
	int q = INT_MIN;
	for(int i = 1; i <= n; ++i)
		q = std::max(p[i - 1], cut_rod(p, n - i));
	return q;
}

int compare_ints(const void* a, const void* b)   // comparison function
{
	int arg1 = *reinterpret_cast<const int*>(a);
	int arg2 = *reinterpret_cast<const int*>(b);
	if(arg1 < arg2) return -1;
	if(arg1 > arg2) return 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//boost.property_map
template <typename AddressMap>
void foo(AddressMap address)
{
	typedef typename boost::property_traits<AddressMap>::value_type value_type;
	typedef typename boost::property_traits<AddressMap>::key_type key_type;

	value_type old_address, new_address;
	key_type fred = "Fred";
	old_address = get(address, fred);
	new_address = "384 Fitzpatrick Street";
	put(address, fred, new_address);

	key_type joe = "Joe";
	value_type& joes_address = address[joe];
	joes_address = "325 Cushing Avenue";
}

//////////////////////////////////////////////////////////////////////////
int test_cti_main(int argc, char* argv[]){
	std::locale::global(std::locale(""));
	if(argc != 3)
		return -1;
	{
		std::string str(argv[1]);
		std::cout << str << ("中字符各不相同为") << all_diff(str.begin(), str.end()) << std::endl;
		std::cout << ("reverse前") << argv[1];
		std::cout << (", reverse后") <<my_reverse(argv[1])<< std::endl;
		std::cout << argv[1] << (", ") << argv[2] << ("元素是否相同:") << elements_same(argv[1], argv[2]) << std::endl;
		std::cout << ("替换") << argv[1] << ("中的") << (" ") << ("为") << ("") ;
		std::cout << replace_char_to_str(argv[1], (' '), ("%20")) << std::endl;
		char src[] = "aabcccccaaa", dest[256] = "";
		compress_repeat_str(src, strlen(src), dest);
		std::cout << ("压缩前:") << src << std::endl << ("压缩后:") << dest << std::endl;

	}
	{

		size_t const count = 256, start = 0, end = 255;
		int arr[count];
		for(size_t i = 0; i != count; ++i)
			arr[i] = roll_die(start, end);

	}
	{
		size_t const count = 256;
		int start = -255, end = 255;
		int arr[count], arr2[count];
		for(size_t i = 0; i != count; ++i)
			arr[i] = roll_die(start, end);

		std::copy(arr, arr + count, arr2);

		std::cout << ("归并排序前:") << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));

		merge_sort(arr, count);

		std::cout << std::endl << ("归并排序后:") << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;

		std::cout << std::endl << ("std::sort()") << std::endl;
		std::sort(arr2, arr2 + count);
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));


		std::cout << std::endl;
	}
	{
		//最大子数组
		int arr[] = {-9, 9, -1, 7, -2, -8, -4, 0, -5, -2};
		size_t const count = sizeof(arr) / sizeof(arr[0]);
		std::cout << "最大子数组: " << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;
		int maxSum = 0;
		size_t low = 0, high = 0;
		maximun_subarray(arr, count, maxSum, low, high);
		std::printf("maxSum=%d, low=%zu, high=%zu\n", maxSum, low, high);
		maxSum = low = high = 0;
		maximun_subarray2(arr, count, maxSum, low, high);
		std::printf("maxSum=%d, low=%zu, high=%zu\n", maxSum, low, high);


	}
	{
		//最大子数组2
		int arr[10];
		size_t const count = sizeof(arr) / sizeof(arr[0]);
		for(int i = 0; i != count; ++i){
			arr[i] = roll_die(-10, 10);
		}
		std::cout << "最大子数组: " << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;
		int maxSum = 0;
		size_t low = 0, high = 0;
		maximun_subarray(arr, count, maxSum, low, high);
		std::printf("maxSum=%d, low=%zu, high=%zu\n", maxSum, low, high);
		maxSum = low = high = 0;
		maximun_subarray2(arr, count, maxSum, low, high);
		std::printf("maxSum=%d, low=%zu, high=%zu\n", maxSum, low, high);

	}
	{
		//二分搜索 
		size_t const count = 32;
		int arr[count];
		for(int i = 0; i != count; ++i){
			arr[i] = roll_die(0, 255);
		}
		std::sort(arr, arr + count);

		std::cout << "二分搜索: " << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;

		int i = roll_die(0, count - 1);
		int const& val = arr[i];
		std::printf("val = %d, index=%d\n", val, binary_search(arr, count, val));		
	}
	{
		//最大公因数
		int m = roll_die(2, 100), n = roll_die(2, 100);
		std::printf("最大公因数gcd(%d, %d) == %ld\n", m, n, gcd(m, n));

	}
	{
		//中缀表达式转后缀表达式
		size_t const count = 256;
		char infix[count] = "a+b*c+(d*e+f)*g+(h-i*j)";
		char postfix[count] = "";
		infix2postfix(infix, postfix);
		std::printf("中缀表达式转后缀表达式\n中缀表达式:%s\n后缀表达式:%s\n", infix, postfix);
		std::string expr = "8 + 5 * 2 + ( 1 + 5) * 2 + 1";
		std::printf("计算算术表达式的值, 限四则运算,可含括号:%s=%f\n", expr.c_str(), calc4(expr));
	}
	{
		//Josephus问题
		size_t const count = 5, n = roll_die(1, 2 * count);
		int childs[count];
		for(size_t i = 0; i != count; ++i)
			childs[i] = roll_die(1, 16);
		std::printf("Josephus问题: m=%zu, n=%zu\n", count, n);
		std::copy(childs, childs + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;
		std::printf("Josephus=%d\n", Josephus(childs, count, n));


	}
	{
		//汉诺塔问题
		int const n = 4;
		std::cout << "汉诺塔问题, 盘子数:" << n << std::endl;
		Hanoi('A', 'B', 'C', n);
	}
	{

	}
	{}
	{}
	{
		std::cout << "boost.property_map:" << std::endl;

		std::map<std::string, std::string> name2address;
		boost::associative_property_map< std::map<std::string, std::string> >
			address_map(name2address);

		name2address.insert(make_pair(std::string("Fred"), 
			std::string("710 West 13th Street")));
		name2address.insert(make_pair(std::string("Joe"), 
			std::string("710 West 13th Street")));

		foo(address_map);

		for (std::map<std::string, std::string>::iterator i = name2address.begin();
			i != name2address.end(); ++i)
			std::cout << i->first << ": " << i->second << "\n";

	}
	{
		//不相交集
		maze(10, 30);
	}
	{}

	return EXIT_SUCCESS;
}	//test_main()

#endif
