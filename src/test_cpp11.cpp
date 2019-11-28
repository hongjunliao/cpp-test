#include <cpp_test.h>
#include <stdio.h>      /*  */
#include <thread>		/*std::thread*/
#include <atomic>		/*std::atomic*/
#include <vector>	    /*std::vector*/
#include <stdarg.h>	    /* va_list */

/*!
 * test for c++11
 * c++并发编程实践： http://blog.csdn.net/column/details/ccia.html
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

/////////////////////////////////////////////////////////////////////////////
//创建了 10 个线程进行计数，率先完成计数任务的线程输出自己的 ID，后续完成计数任务的线程不会输出自身 ID
std::atomic<bool> g_is_ready(false);    // can be checked without being set
std::atomic_flag g_winner = ATOMIC_FLAG_INIT;    // always set when checked

static void count1m(int id)
{
    while (!g_is_ready) {
        std::this_thread::yield();
    } // 等待主线程中设置 ready 为 true.

    size_t sum = 0;
    for (size_t i = 0; i < (size_t)100000000 * 5; ++i) {
    	sum += i % 10 == 0? i * 2 : i;
    } // 计数.

    // 如果某个线程率先执行完上面的计数过程，则输出自己的 ID.
    // 此后其他线程执行 test_and_set 是 if 语句判断为 false，
    // 因此不会输出自身 ID.
    if (!g_winner.test_and_set()) {
        fprintf(stdout, "%s: thread #%d won!sum=%zu\n", __FUNCTION__, id, sum);
    }
};

static int test_atomic_flag(int argc, char ** argv)
{
	std::vector<std::thread> threads;
	fprintf(stdout, "%s: spawning 10 threads that count to 1 million...\n", __FUNCTION__);
	for (int i = 1; i <= 10; ++i)
		threads.push_back(std::thread(count1m, i));
	g_is_ready = true;

	for (auto & th:threads)
		th.join();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static std::atomic_ulong g_line_count  = { 0 };
static int test_atomic(int argc, char ** argv)
{
	std::atomic_ulong a  = { 0 };
	g_line_count ++;

	std::atomic_flag f = ATOMIC_FLAG_INIT;
	return g_line_count;
}
/////////////////////////////////////////////////////////////////////////////
//c++11 unicode
static int test_cpp11_unicode(int argc, char ** argv)
{
	char anscii_str[] = "hello";
	char utf8[] =  u8"\u4F60\u597D\u554A"; //你好啊
	char16_t utf16[] = u"hello";
	char32_t utf32[] = U"hello";
	fprintf(stdout, "%s: ancii=%s, utf8=%s, utf16=%s, utf32=%s\n", __FUNCTION__,
			anscii_str, utf8, utf16, utf32);
	return 0;
}

int test_vsscanf_main(int argc, char ** argv)
{
	char buf[] = "23 345 3 2";

	va_list args;
	vsscanf(buf, "%d%d%d%d", args);
}

struct s_with_array {
	int arr[3];
};

/////////////////////////////////////////////////////////////////////////////
class sizeof_a {
//	int a;
//	static int b;
//	virtual void func();
};

class sizeof_b : public sizeof_a {
	int c;
	virtual void func();
//	char d;
//	double e;
};

static int test_sizeof_class_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: sizeof_a=%zu, sizeof_b=%zu\n", __FUNCTION__,
			sizeof(sizeof_a), sizeof(sizeof_b));
	return 0;
}

int test_cpp11_main(int argc, char ** argv)
{
	test_sizeof_class_main(argc, argv);
	s_with_array a, b;
	b = a;

	int arr[3] = {3};
	for(int i = 0; i < 3; ++i)
		fprintf(stdout, "%s: arr[%d] = %d\n", __FUNCTION__, i, arr[i]);
	test_lamda(argc, argv);
	test_atomic(argc, argv);
//	test_atomic_flag(argc, argv);
	test_cpp11_unicode(argc, argv);
	std::initializer_list<int> i {3, 4, 5};
	return 0;
}

