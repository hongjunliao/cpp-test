/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * binary_search_tree
 * */
//////////////////////////////////////////////////////////////////////////
#ifndef TEST_CRACKING_THE_CODING_INTERVIEW_BSTREE
#define TEST_CRACKING_THE_CODING_INTERVIEW_BSTREE
#include <iostream> /* std::cout, cin, std::endl */
/////////////////////////////////////////////////////////////////////////
//二叉搜索/查找树, 性质:对于树中的每个结点X, 其左子树中的所有项的值小于X中的项,
//而它的右子树中的所有项的值大于X中的项
//平均深度:O(logN)
template <typename Comparable>
class BinarySearchTree
{
private:
	struct BinaryNode
	{
		Comparable element_;
		BinaryNode *left_;
		BinaryNode *right_;

		BinaryNode( const Comparable & theElement, BinaryNode *lt, BinaryNode *rt )
			: element_( theElement ), left_( lt ), right_( rt ) { }
	};
public:
private:
	BinaryNode *root_;

public:
	BinarySearchTree( );
	BinarySearchTree( const BinarySearchTree & rhs );
	~BinarySearchTree( );
public:
	Comparable * findMin( ) const;
	Comparable * findMax( ) const;
	bool contains( const Comparable & x ) const;
	bool isEmpty( ) const;
	int height() const;
	void printTree(std::ostream & os = std::cout) const;

	void makeEmpty( );
	void insert( const Comparable & x );
	void remove( const Comparable & x );
public:
	BinarySearchTree & operator=( BinarySearchTree const& rhs );
public:
	inline BinaryNode * const& root() const { return root_; }

private:
	void insert( const Comparable & x, BinaryNode * & t ) const;
	void remove( const Comparable & x, BinaryNode * & t ) const;
	BinaryNode * findMin( BinaryNode *t ) const;
	BinaryNode * findMax( BinaryNode *t ) const;
	bool contains( const Comparable & x, BinaryNode *t ) const;
	int height(BinaryNode *t) const;

	void makeEmpty( BinaryNode * & t );
	void printTree( BinaryNode *t, std::ostream & os) const;
	BinaryNode * clone(BinaryNode *t ) const;
};
template class BinarySearchTree<int>;
template class BinarySearchTree<float>;
//////////////////////////////////////////////////////////////////////////
typedef BinarySearchTree<int> IntBSTree;
typedef BinarySearchTree<float> FloatBSTree;
//////////////////////////////////////////////////////////////////////////
//检查二叉棵树是否平衡
template<typename T, typename Node>
bool is_balanced(T const& t, Node const* n = NULL);
//求结点高度
template<typename T, typename Node>
int btree_height(T const& t, Node const* n);

#endif /* TEST_CRACKING_THE_CODING_INTERVIEW_BSTREE */
