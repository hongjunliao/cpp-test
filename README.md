## algorithms and tests for C/C++

代码主要源于个人日常学习及工作积累. 经典面试题,较复杂的数据结构及算法(如红黑树,图算法,字符串等)的C/C++实现.
注释标明了其源自的书名及章节,书目:<算法第4版>,<算法导论第3版>,<数据结构与算法分析(C++描述)>,<程序员面试经典第5版>,<剑指Offer>.
如果你正阅读以上书籍,相应代码会让你感到熟悉

	1.algs/ 算法(从2-3查找树到红黑树,图的表示,初始化及遍历,字符串算法如单词查找树);
		9787302356288/ 算法竞赛入门经典（第2版） by 刘汝佳
		algorithms4th/  <算法 第4版> 美 Robert Sedgewick / 美Kevin Wayne编写
		cti/ 数据结构与算法分析(C++描述)_第３版/Cracking_The_Interview,常见数据结构及算法题
		intro_to_algs3th/ <算法导论3>
		jzoffer/ <剑指Offer>相关章节
	2.chess/: 五子棋(目前仅实现判定算法,暂无AI)
	3.tests/: 库及系统调用的测试  
	4.win32/: 实现了一个基于管道的win32进程IO重定向(实时捕获另一个进程的输出) 
	5.mem_pool: 一个小型对象(sizeof返回1024以下)分配池实现
	6.paxos/: the paxos protocol test, see paxos.jpg
	
## 如何阅读代码及使用

所有测试均以类似test_xxx_main函数为入口(参见plcdn_cpp_test.cpp以查看所有测试, 有些测试仅适用于单个平台). 
你可以从这些入口函数开始阅读,如果你想运行其中的测试,可使用如下方式:

	extern int test_xxx_main(int argc, char ** argv);
	int main(int argc, char ** argv)
	{
    	return test_xxx_main(argc, argv);
	}

这是为了可以使用如下方式运行测试:
$ ./mytest rbtree_insert arg1 arg2 ...

## build

cd gcc_shared; make all;