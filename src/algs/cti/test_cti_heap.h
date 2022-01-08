/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 二叉堆
 * */
//////////////////////////////////////////////////////////////////////////
#ifndef TEST_CRACKING_THE_CODING_INTERVIEW_HEAP
#define TEST_CRACKING_THE_CODING_INTERVIEW_HEAP
#include <queue>               /* std::queue */
#include <stack>               /* std::stack */
#include <map>    /* std::map */
#include <string> /* std::string */
#include <vector> /* std::vector */
//二叉堆
//结构性质:是一棵完全填满的二叉树,可能的例外是在最底层, 底层上的元素从左到右填入
//堆序性质:对于任意结点X, X的父亲中的键<=X中的键,根结点除外(它没有父亲)
template <typename Comparable>
class BinaryHeap
{
public:
	explicit BinaryHeap( int capacity = 100 );
	explicit BinaryHeap( const std::vector<Comparable> & items );
public:
	bool isEmpty( ) const;
	const Comparable & findMin( ) const;

	void insert( const Comparable & x );
	void deleteMin( );
	void deleteMin( Comparable & minItem );
	void makeEmpty( );
	size_t capacity() const;
	template<typename Bh>
	bool same(Bh const& q) const;
private:
	std::vector<Comparable> array_;        // The heap array

	void buildHeap( );
	void percolateDown( int hole );
};

template class BinaryHeap<int>;
//////////////////////////////////////////////////////////////////////////
//维护最大堆的性质, 这里假定左子树2 * i, 右子树2 * i + 1(如果有的话)均为最大堆
//@param a 数组, @param size 数组大小, @param i 下标
//时间复杂度:O(h)
template<typename T>
void max_heapify(T& a, int size, int i);
//建堆(最大堆),
//时间复杂度:O(n)
template<typename T>
void build_max_heap(T& a, int size);
//堆排序
//时间复杂度:O(nlogn)
template<typename T>
void heap_sort(T& a, int size);
#endif /* TEST_CRACKING_THE_CODING_INTERVIEW_HEAP */
