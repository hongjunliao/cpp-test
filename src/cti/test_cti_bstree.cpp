/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * binary_search_tree
 * */
//////////////////////////////////////////////////////////////////////////
#ifdef WITH_BSTREE
#include "test_bstree.h"
#include <stdlib.h>
#include <algorithm>

extern int roll_die(int start, int end);
//////////////////////////////////////////////////////////////////////////
//二叉搜索树, 实现
template <typename Comparable>
BinarySearchTree<Comparable>::BinarySearchTree()
	:	root_(NULL)
{
}
template <typename Comparable>
BinarySearchTree<Comparable>::~BinarySearchTree()
{
	makeEmpty();
}

template <typename Comparable>
Comparable * BinarySearchTree<Comparable>::findMin( ) const{
	return root_? &findMin(root_)->element_: NULL;
}

template <typename Comparable>
Comparable * BinarySearchTree<Comparable>::findMax( ) const{
	return root_? &findMin(root_)->element_: NULL;
}
template <typename Comparable>
typename BinarySearchTree<Comparable>::BinaryNode *
	BinarySearchTree<Comparable>::findMin( BinaryNode *t ) const{
		if(t->left_)
			return findMin(t->left_);
		else
			return t;
}
template <typename Comparable>
typename BinarySearchTree<Comparable>::BinaryNode *
	BinarySearchTree<Comparable>::findMax( BinaryNode *t ) const{
		while(t->right_)
			t = t->right_;
		return t;
}


template <typename Comparable>
bool BinarySearchTree<Comparable>::contains( const Comparable & x ) const
{
	return contains(x, root_);
}
template <typename Comparable>
void BinarySearchTree<Comparable>::insert( const Comparable & x ){
	return insert(x, root_);
}

template <typename Comparable>
void BinarySearchTree<Comparable>::remove( const Comparable & x ){
	return remove(x, root_);
}
template <typename Comparable>
void BinarySearchTree<Comparable>::makeEmpty( ){
	return makeEmpty(root_);
}

template <typename Comparable>
void BinarySearchTree<Comparable>::makeEmpty( BinaryNode * & t )
{
	if(!t)
		return;
	makeEmpty(t->left_);
	makeEmpty(t->right_);
	delete t;
	t = NULL;
}

template <typename Comparable>
void BinarySearchTree<Comparable>::insert( const Comparable & x, BinaryNode * & t ) const{
	if(!t)
		t = new BinaryNode(x, NULL, NULL);
	else if(x < t->element_)
		return insert(x, t->left_);
	else if( t->element_ < x)
		return insert(x, t->right_);
	else
		;
}

template <typename Comparable>
void BinarySearchTree<Comparable>::remove( const Comparable & x, BinaryNode * & t ) const{
	if(!t)
		return;
	if( x < t->element_)
		remove(x, t->left_);
	else if( x > t->element_)
		remove(x, t->right_);
	else if(!t->left_ && !t->right_){	//2 children
		t->element_ = findMin(t->right_)->element_;
		remove(t->element_, t->right_);
	}
	else {
		BinaryNode * oldNode = t;
		t = (t->left_? t->left_ : t->right_);
		delete oldNode;
	}
}


template <typename Comparable>
bool BinarySearchTree<Comparable>::contains( const Comparable & x, BinaryNode *t ) const
{
	if(!t)
		return false;
	else if(x < t->element_)
		return contains(x, t->left_);
	else if( t->element_ < x)
		return contains(x, t->right_);
	else
		return true;
}
template <typename Comparable>
BinarySearchTree<Comparable> &
	BinarySearchTree<Comparable>::operator=( BinarySearchTree const & rhs )
{
	if(false){
		makeEmpty();
		root_ = clone(rhs.root_);
	}
	return *this;
}
template <typename Comparable>
bool BinarySearchTree<Comparable>::isEmpty( ) const { return NULL == root_ ; }

template <typename Comparable>
typename BinarySearchTree<Comparable>::BinaryNode *
	BinarySearchTree<Comparable>::clone( BinaryNode *t ) const{
		if(!t)
			return NULL;
		return new BinaryNode(t->element_, clone(t->left_), clone(t->right_));
}
template <typename Comparable>
void BinarySearchTree<Comparable>::printTree( BinaryNode *t, std::ostream & os) const
{
	if(t){
		printTree(t->left_, os);
		os << t->element_ << ", ";
		printTree(t->right_, os);
	}
}
template <typename Comparable>
int BinarySearchTree<Comparable>::height() const
{
	return height(root_);
}
template <typename Comparable>
int BinarySearchTree<Comparable>::height(BinaryNode *t) const
{
	if(!t)
		return -1;
	else
		return 1 + std::max(height(t->left_), height(t->right_));
}

template <typename Comparable>
void BinarySearchTree<Comparable>::printTree(std::ostream & os) const
{
	if(isEmpty())
		os << "Empty tree" << std::endl;
	else
		printTree(root_, os);
}
//////////////////////////////////////////////////////////////////////////
template<typename T, typename Node>
bool is_balanced(T const& t, Node const* n){
	return NULL == n || std::abs(btree_height(t, n->left_) - btree_height(t, n->right_)) < 2;
}
template<typename T, typename Node>
int btree_height(T const& t, Node const* n){
	return NULL != n? 1 + std::max(btree_height(t, n->left_), btree_height(t, n->right_)) : 0;
}

int test_cti_bstree_main(int argc, char* argv[])
{
	//二叉搜索树
	IntBSTree ibtree;
	FloatBSTree fbtree;

	int data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	size_t const count = sizeof(data) / sizeof(*data);
	for(int i = 0; i != count; ++i)
		std::swap(data[i], data[roll_die(0, i)]);

	for(int i = 0; i != count; ++i)
		ibtree.insert(data[i]);
	std::cout << "二叉搜索树" << std::endl;
	ibtree.printTree(); std::cout << std::endl;
	std::cout << "是否平衡is_balanced:" << is_balanced(ibtree, ibtree.root()) << std::endl;
}

#endif /* WITH_BSTREE */
