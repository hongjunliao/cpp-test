/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 二叉堆
 * */
#include "test_cti.h"
#include "test_cti_heap.h"
#include <limits.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>            /* std::cout, cin, std::endl */
#include <algorithm>		   /* std::copy */
#include <iterator>		       /* std::ostream_iterator */

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
//二叉堆, 实现
template<typename Comparable>
BinaryHeap<Comparable>::BinaryHeap( int capacity )
{
	array_.reserve(capacity);
	array_.push_back(0);
}
template<typename Comparable>
BinaryHeap<Comparable>::BinaryHeap( const std::vector<Comparable> & items )
{
	array_ = items;
	buildHeap();
}

template<typename Comparable>
void BinaryHeap<Comparable>:: deleteMin( ){
	if(isEmpty())
		return;
	percolateDown(1);
	array_.pop_back();
}
template<typename Comparable>
void BinaryHeap<Comparable>:: buildHeap( ){
	if(isEmpty())
		return;
	for(size_t i = array_.size() / 2; i > 0; --i)
		percolateDown(i);
}

template<typename Comparable>
void BinaryHeap<Comparable>::percolateDown( int i ){
	for(; 2 * i + 1 < array_.size();){
		if(array_[2 * i] < array_[2 * i + 1]){
			array_[i] = array_[2 * i];
			i = i * 2;
		}
		else{
			array_[i] = array_[2 * i + 1];
			i = i * 2 + 1;
		}
	}
	array_[i] = array_.back();
}

template<typename Comparable>
void BinaryHeap<Comparable>::deleteMin( Comparable & minItem ){
	if(isEmpty())
		return;
	minItem = array_[1];
	percolateDown(1);
	array_.pop_back();
}
template<typename Comparable>
bool BinaryHeap<Comparable>::isEmpty() const{
	return array_.size() <= 1;
}


template<typename Comparable>
void BinaryHeap<Comparable>::insert( const Comparable & x )
{
	array_.push_back(x);
	int i = array_.size() - 1;
	for(; array_[i / 2] > x; i /= 2){
		array_[i] = array_[i / 2];
	}
	array_[i] = x;
}
template<typename Comparable>
size_t BinaryHeap<Comparable>::capacity() const
{
	return array_.capacity();
}
template<typename Comparable>
template<typename Bh>
bool BinaryHeap<Comparable>::same(Bh const& q) const
{
	Bh stdq(q);
	for(size_t i = 1; i < array_.size(); ++i){
		int const& k = stdq.top();
		if(array_[i] != k)
			return false;
		stdq.pop();
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
template<typename T>
void max_heapify(T& a, int size, int i){
	int largest = i;
	if(2 * i < size && a[2 * i] > a[i])
		largest = 2 * i;
	if(2 * i + 1 < size && a[2 * i + 1] > a[largest])
		largest = 2 * i + 1;
	if(largest != i){
		std::swap(a[largest], a[i]);
		max_heapify(a, size, largest);
	}
}
template<typename T>
void build_max_heap(T& a, int size){
	for(int i = size / 2 - 1; i >= 0; --i)
		max_heapify(a, size, i);
}
template<typename T>
void heap_sort(T& a, int size){
	build_max_heap(a, size);
	for(int i = size - 1; i > 0; --i){
		std::swap(a[i], a[0]);
		max_heapify(a, --size, 0);
	}
}

template<typename T>
int quicksort_partition(T & a, int p, int r){
	int i = p - 1;
	for(int j = p; j != r; ++j){
		if(a[j] <= a[r]){
			++i;
			std::swap(a[i], a[j]);
		}
	}
	std::swap(a[i + 1], a[r]);
	return i + 1;
}
template<typename T>
void quicksort(T & a, int p, int r){
	if(p < r){
		int q = quicksort_partition(a, p, r);
		quicksort(a, p, q - 1);
		quicksort(a, q + 1, r);
	}
}

int test_cti_heap_main(int argc, char* argv[])
{

	//二叉堆
	std::vector<int> vecHeap = {8, 2, 5, 9, 6, 4, 7, 1 },
		v2 = vecHeap, v3 = vecHeap;
	BinaryHeap<int> h(10), h2(vecHeap);

	size_t const capacity = vecHeap.size();
	for(size_t i = 0; i != capacity; ++i){
		int rand = vecHeap[i];
		h.insert(rand);
	}
	std::make_heap(vecHeap.begin(), vecHeap.end());
	while(!h.isEmpty()){
		int min = 0;
		h.deleteMin(min);
		std::cout << min << ", ";
	}
	std::cout << std::endl;

	//
	std::make_heap(v2.begin(), v2.end());
	std::sort_heap(v2.begin(), v2.end());

	std::cout << "堆排序:\nstd::make_heap/std::sort_heap:\n";
	std::copy(v2.begin(), v2.end(), std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;

	build_max_heap(v3, v3.size());
	heap_sort(v3, v3.size());

	std::cout << "build_max_heap/heap_sort:\n";
	std::copy(v3.begin(), v3.end(), std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;

	int v4[] = { -2, 99, 0, -743, 2, 3, 4 };
	size_t const size = sizeof(v4) / sizeof(v4[0]);
	int v5[size];
	std::copy(v4, v4 + size, v5);

	std::qsort(v4, size, sizeof(v4[0]), compare_ints);
	quicksort(v5, 0, size - 1);

	std::cout << "快速排序:\nstd::qsort:\n";
	std::copy(v4, v4 + size, std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;

	std::cout << "快速排序:\nquicksort:\n";
	std::copy(v5, v5 + size, std::ostream_iterator<int>(std::cout, ", "));
	std::cout << std::endl;


}
