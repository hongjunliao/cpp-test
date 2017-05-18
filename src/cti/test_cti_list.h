/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 双链表
 * */
//////////////////////////////////////////////////////////////////////////
#ifndef TEST_CRACKING_THE_CODING_INTERVIEW_LIST
#define TEST_CRACKING_THE_CODING_INTERVIEW_LIST

#include <cstdio>
#include <cstdlib>

template<typename Key>
class list_node{
public:
	typedef Key value_type;
public:
	Key key_;
	list_node * prev_, * next_;
public:
	list_node(Key const& k = Key()): key_(k), prev_(NULL), next_(NULL) {}
};

template<typename Key>
class my_list{
public:
	list_node<Key> * head_;
public:
	my_list(): head_(NULL){}
};
//带哨兵的list
template<typename Key>
class list_with_sentinel{
public:
	list_node<Key> nil_;	
public:
	list_with_sentinel();
};
//
template class list_node<int>;
template class my_list<int>;
//
template class list_with_sentinel<int>;
//
//双链表查找
template<typename Key>
list_node<Key>* list_search(my_list<Key> const& lst, Key const& k);
//双链表插入
template<typename Key>
void list_insert(my_list<Key> & lst, list_node<Key> const& node);
//双链表删除
template<typename Key>
void list_delete(my_list<Key> & lst, Key const& k);
//带哨兵的list的操作
template<typename Key>
list_node<Key>* list_search(list_with_sentinel<Key> const& lst, Key const& k);
template<typename Key>
void list_insert(list_with_sentinel<Key> & lst, list_node<Key> const& node);
template<typename Key>
void list_delete(list_with_sentinel<Key> & lst, Key const& k);

#endif /* TEST_CRACKING_THE_CODING_INTERVIEW_LIST */
