/*!
 * 数据结构与算法分析(C++描述)_第３版
 * */

#if 0
#include "boost/pending/disjoint_sets.hpp"	//不相交集
#include "boost/unordered_map.hpp"
#include "boost/graph/prim_minimum_spanning_tree.hpp"	//prim算法, 最小生成树
namespace test{
typedef std::string tstring;
//////////////////////////////////////////////////////////////////////////
//汉诺塔问题:将@n个盘子从@a移动到@c, 
//期间要求所有柱子(@a, @b, @c)上的盘子小的在上, 可使用@b 
void Hanoi(char a, char b, char c, int n);
void my_strcpy(char* from, char const* to);
//归并排序, length为数组长度
void merge_sort(int * arr, size_t length);
//归并己排序数组
//满足p << q < r, arr[p..q], arr[q + 1..r]己排好序, p, q, r 为数组下标
void my_merge(int * arr, size_t p, size_t q, size_t r);
//***********************
//利用字符出现的字数, 实现基本的压缩, 如输入"aabcccccaaa", 输出"a2b1c5a3"
void compress_repeat_str(char const* src, size_t length, char* dest);
//若MxN矩阵中某个元素为0, 则将其所在行列清0
void clear_matrix(int** matrix, size_t m, size_t n, int val = 0);
//比较[begin, end)区间字符各不相同
bool all_diff(tstring::const_iterator begin, tstring::const_iterator end);
//反转nulll结尾的字符串
char * my_reverse(char * str);
//比较元素是否相同
bool elements_same(tstring const& a, tstring const& b);
//替换字符为指定字符串
char* replace_char_to_str(char* src, char const& c, char const* str);
//插入排序
void insertion_sort(int * arr, size_t length);
//产生随机数
int roll_die(int start, int end);
//最大子数组
void maximun_subarray(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high);
//最大子数组2,含求low, high
void maximun_subarray2(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high);
//二分搜索, @arr已排好序
int binary_search(int const* arr, int length, int val);
//欧里几得算法求最大公因数
long gcd(long m, long n);
//中缀表达式转后缀表达式
//@expre 中缀表达式, @postfix 后缀表达式
//例 a + b * c + (d * e + f) * g 转 a b c * + d e * f + g * +
void infix2postfix(char const* infix, char* postfix);
//比较运算符优先级(限'+', '-', '*', '/', '('), 
//如果inputOpa > stackOpb 返回true
// '+'  == '-' < '*' == '/'
//特别地对'(', 若处栈中(stackOpb == '(')则优先级最低
bool higher_priority(char inputOpa, char stackOpb);
//计算算术表达式的值, 限四则运算,可含括号
//不含语法检查
double calc4(std::string const& exp);
//Josephus问题
template<typename T> T Josephus(T* child, int n, int m);
//////////////////////////////////////////////////////////////////////////
class maze_cell{
public:
	int row_, col_;
	bool edge_[4];
};
static inline bool operator!=(maze_cell const& lhs, maze_cell const& rhs){
	return lhs.row_ != rhs.row_ && lhs.col_ != rhs.col_;
}

//不相交集应用: 迷宫生成, boost.disjoint_sets实现 □ ■
void maze(int m, int n);
//////////////////////////////////////////////////////////////////////////
//输出数组
std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list);
//////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////
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

//快速排序:期望时间复杂度为theta(nlgn)
//划分函数
template<typename T>
int quicksort_partition(T & a, int p, int r);
//快速排序
template<typename T>
void quicksort(T & a, int p, int r);
//std::qsort比较函数
int compare_ints(const void* a, const void* b);
//////////////////////////////////////////////////////////////////////////

//双链表
template<typename Key>
class list_node{
public:
	typedef typename Key value_type;
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
	template<typename Key>
		friend list_node<Key>* list_search(list_with_sentinel<Key> const& lst, Key const& k);
	template<typename Key>
		friend void list_insert(list_with_sentinel<Key> & lst, list_node<Key> const& node);
	template<typename Key>
		friend void list_delete(list_with_sentinel<Key> & lst, Key const& k);

protected:
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


//////////////////////////////////////////////////////////////////////////
//动态规划:钢条切割问题
//@param p, 价格数组
//@param n 长度
//@param(out) it 切割方案
//自顶向下的递归实现
int cut_rod(int const* p, int n);
//带备忘的自底向上实现
int bottom_up_cut_rod(int const* p, int n);
template<typename InsertIerator>
int bottom_up_cut_rod(int const* p, int n, InsertIerator it);
//////////////////////////////////////////////////////////////////////////
//图算法
//顶点
class my_vertex{
public:
	enum Color{White, Gray, Black};
public:
	Color color_;	//颜色
	int d_, f_;		//发现时间, 完成时间(用于深度优先搜索)
	int dist_;				//距离(用于广度优先搜索)
	my_vertex const* p_;	//父亲(用于广度优先搜索)
public:
	my_vertex();
};
////////////////////////////////////////////////////////////////////////
//拓扑排序用, 衣服
class clothes_vertex: public my_vertex{ 
public: 
	std::string name_;
public:
	clothes_vertex(std::string const& name = "noname");
};
//////////////////////////////////////////////////////////////////////////
typedef std::map<int, std::vector<int> > dag_graph_t;	//有向无环图
//广度优先搜索
void breadth_first_search(dag_graph_t const& g, int s);
//广度优先搜索, Graph使用boost.graph
template<typename Graph>
void breadth_first_search(Graph& g, typename Graph::vertex_descriptor const& s);
//打印广度优先树
template<typename Graph, typename Vertex>
void print_path(Graph const& g, Vertex const& s, Vertex const& v);

//深度优先搜索, 递归版本
template<typename Graph>
void depth_first_search(Graph& g);
//深度优先搜索, 非递归版本
template<typename Graph>
void depth_first_search_non_recursive(Graph& g);
//////////////////////////////////////////////////////////////////////////
//深度优先搜索应用--拓扑排序
template<typename Graph, typename InsertIerator>
void topological_sort(Graph& g, InsertIerator iter);
//////////////////////////////////////////////////////////////////////////
class custom_bfs_visitor: public boost::default_bfs_visitor{

};
template < typename TimeMap > class bfs_time_visitor:public boost::default_bfs_visitor {
	typedef typename boost::property_traits < TimeMap >::value_type T;
public:
	bfs_time_visitor(TimeMap tmap, T & t):m_timemap(tmap), m_time(t) { }
public:
	template < typename Vertex, typename Graph >
	void discover_vertex(Vertex u, const Graph & g) const
	{
		put(m_timemap, u, m_time++);
	}
	TimeMap m_timemap;
	T & m_time;
};



//
class custom_dfs_visitor : public boost::default_dfs_visitor 
{ 
public: 
	template < typename Vertex, typename Graph >
	void discover_vertex(Vertex u, const Graph & g) const { 
		std::cout << "(" << u << " "; 
	}
	template < typename Vertex, typename Graph >
	void finish_vertex(Vertex u, const Graph & g) const { 
		std::cout << " " << u << ")"; 
	}
	template < typename Vertex, typename Graph >
	void start_vertex(Vertex u, const Graph & g) const { 
		std::cout << endl;
	}
}; 	
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

} //	namespace test

#endif
