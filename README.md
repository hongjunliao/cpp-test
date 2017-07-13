## algorithms and tests for C/C++

代码主要源于个人日常学习及工作积累. 经典面试题,较复杂的数据结构及算法(如红黑树,图算法,字符串等)的C/C++实现,
分布于目录 algs/, jzoffer/, cti/, 注释标明了其源自的书名及章节, 
参考书目:<算法第4版>,<算法导论第3版>,<程序员面试经典第5版>,<剑指Offer>, 
如果你正阅读以上书籍,相应代码会让你感到熟悉

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