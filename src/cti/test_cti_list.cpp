/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 双链表
 * */
//////////////////////////////////////////////////////////////////////////
#include "test_cti_list.h"

template<typename Key>
list_node<Key>* list_search(my_list<Key> const& lst, Key const& k){
	list_node<Key> * x = lst.head_;
	while(x && x->key_ != k)
		x = x->next_;
	return x;
}
template<typename Key>
void list_delete(my_list<Key> & lst, Key const& k){
	list_node<Key> * x = list_search(k);
	if(!x)
		return;
	if(x->prev_)
		x->prev_->next_ = x->next_;
	else
		lst.head_ = x->next_;
	if(x->next_)
		x->next_->prev_ = x->prev_;
	delete x;
	return;
}
template<typename Key>
void list_insert(my_list<Key> & lst, list_node<Key> const& node){
	list_node<Key>* x= new list_node<Key>(node);
	x->next_ = lst.head_;
	if(lst.head_)
		lst.head_->prev_ = x;
	lst.head_ = x;
	x->prev_ = NULL;
}
template<typename Key>
list_with_sentinel<Key>::list_with_sentinel() {
	nil_.next_ =  nil_.prev_ = &nil_;
}

template<typename Key>
list_node<Key>* list_search(list_with_sentinel<Key> const& lst, Key const& k){
	list_node<Key> * x = lst.nil_.next_;
	while(x != &lst.nil_ && x->key_ != k)
		x = x->next_;
	return x;
}
template<typename Key>
void list_insert(list_with_sentinel<Key> & lst, list_node<Key> const& node){
	list_node<Key>* x= new list_node<Key>(node);
	x->next_ = lst.nil_.next_;
	x->prev_ = &lst.nil_;
	lst.nil_.next_ = x;
	lst.nil_.next_->prev_ = x;
}
template<typename Key>
void list_delete(list_with_sentinel<Key> & lst, Key const& k){
	list_node<Key> * x = list_search(lst, k);
	if(!x)
		return;
	x->prev_->next_ = x->next_;
	x->next_->prev_ = x->prev_;
	delete x;
}

int test_list_main(int argc, char* argv[])
{
	//双链表
	my_list<int> lst;
	list_node<int> node(8);
	list_insert(lst, node);
	list_node<int>* e = list_search(lst, 5);
	e = list_search(lst, 8);

	list_with_sentinel<int> lst2;
	list_insert(lst2, node);
	e = list_search(lst2, 5);
	list_delete(lst2, 8);
}
