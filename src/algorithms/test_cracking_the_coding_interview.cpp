#if 0
#include "test_cracking_the_coding_interview.h"
namespace test{
using std::cout;
using std::endl;
//////////////////////////////////////////////////////////////////////////
void clear_matrix(int** matrix, size_t m, size_t n, int val){
	for(size_t i = 0; i != m; ++i){
		for(size_t j = 0; j != n; ++j){
			if(matrix[i][j] == val){

			}
		}
	}
}
void Hanoi(char a, char b, char c, int n){
	if(1 == n)	//a上只有一个盘子了
		cout << n << ": " << a << " -> " << c << endl;
	else{
		Hanoi(a, c, b, n - 1);	//将a上n-1个盘子移动到b
		cout << n << ": " << a << " -> " << c << endl;
		Hanoi(b, a, c, n - 1);	
	}
}
void my_strcpy(char* from, char const* to){
	while(*to)
		*from++ = *to++;
	*from = 0;
}
void merge_sort(int * arr, size_t length){
	if(!arr || length < 2)
		return;
	size_t q = length / 2;
	merge_sort(arr, q);		//注意第二个参数为数组长度!
	merge_sort(arr + q, length - q);
	my_merge(arr, 0, q - 1, length - 1);
}

void my_merge(int * arr, size_t p, size_t q, size_t r)
{
	size_t const n1 = q - p + 1, n2 = r - q;
	int *L = new int[n1 + 1], *R = new int[n2 + 1];
	std::copy(arr, arr + n1, L);
	std::copy(arr + n1, arr + n1 + n2, R);

	L[n1] = INT_MAX, R[n2] = INT_MAX;	//哨兵

	size_t i = 0, j = 0;
	for(size_t k = p; k <= r; ++k){
		if(L[i] <= R[j]){
			arr[k] = L[i];
			++i;
		}
		else{
			arr[k] = R[j];
			++j;
		}
	}
	delete[] L;
	delete[] R;
}


void compress_repeat_str(char const* src, size_t length, char* dest){
	if(!src || 0 == length)
		return ;
	char const* beg = src, *end = src + length;
	size_t index = 0;
	char c = *beg++;
	size_t count = 1;
	for(; beg != end; ++beg){
		if(c == *beg)
			++ count;
		else{
			index += sprintf(dest + index, "%c%u", c, count);;
			c = *beg;
			count = 1;
		}
	}
	index += sprintf(dest + index, "%c%u", c, count);;
	if(index >= length)
		memcpy(dest, src, length);
}
bool all_diff(tstring::const_iterator begin, tstring::const_iterator end)
{
	if(begin == end)
		return true;
	char c = *begin;
	++begin;
	for(tstring::const_iterator iter = begin; iter != end; ++iter){
		if(*iter == c)
			return false;
	}
	return all_diff(begin, end);
}
char * my_reverse(char * str)
{
	size_t length = strlen(str), half = length / 2;
	for(size_t i = 0; i < half; ++i){
		if(i == length - i)
			return str;
		auto c = str[i];
		str[i] = str[length - i - 1];
		str[length - i -  1] = c;
	}
	return str;
} 
bool elements_same(tstring const& a, tstring const& b){
	std::map<char, size_t> charsa, charsb;
	for(tstring::const_iterator iter = a.begin(); iter != a.end(); ++iter)
		++charsa[*iter];
	for(tstring::const_iterator iter = b.begin(); iter != b.end(); ++iter)
		++charsb[*iter];
	return charsa == charsb;
}
char* replace_char_to_str(char* src, char const& c, char const* str){
	tstring const s = src;
	size_t length = _tcslen(str);
	for(size_t i = 0; i != s.size(); ++i){
		if(s[i] == c){
			_tcscpy(src, str);
			src += length;
		}
		src[i] = s[i];
	}
	return src;
}

void insertion_sort(int * arr, size_t length){
	if(length < 2 || !arr)
		return;
	for(size_t j = 1; j != length; ++j){
		int i = j - 1, key = arr[j];	//这里i有可能值为负数, arr[j]必须被暂存下来
		for(; i >= 0 && arr[i] > key; --i)
			arr[i + 1] = arr[i];
		arr[i + 1] = key;
	}
}
int roll_die(int start, int end){
	static boost::random::mt19937 gen(std::time(NULL));
	boost::random::uniform_int_distribution<> dist(start, end);
	return dist(gen);
}
void maximun_subarray(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high){
	if(!arr || 0 == length) return;
	int b = 0;
	maxSum = 0;
	for(size_t i = 0; i != length; ++i){
		if(b < 0)
			b = arr[i];
		else 
			b += arr[i];
		if(maxSum < b)
			maxSum = b;
	}
	return;
}
void maximun_subarray2(int const* arr, size_t length, int& maxSum, size_t& low, size_t& high){
	if(!arr || 0 == length) 
		return;
	maxSum =  low = high = 0;
	int thisSum = 0;
	bool lastInc = false;
	for(size_t i = 0; i != length; ++i){
		thisSum += arr[i];
		lastInc = (thisSum > maxSum);
		if(lastInc){
			high = i;
			maxSum = thisSum;
		}
		else{
			if(thisSum < 0)
				thisSum = 0;
		}
	}
	thisSum = maxSum; 
	int i = high;
	for(; thisSum != 0; --i)
		thisSum -= arr[i];
	low = i + 1;
	return;
}

int binary_search(int const* arr, int length, int val){
	if(!arr || 0 == length) 
		return -2;
	int low = 0, high = length;
	while(low <= high){
		int mid = (low + high) / 2;
		if(arr[mid] < val)
			low = mid + 1;
		else if(arr[mid] > val)
			high = mid -1;
		else
			return mid;
	}
	return -1;
}
long gcd(long m, long n){
	if(n > m)
		std::swap(m, n);
	while(n != 0){
		long rem = m % n;
		m = n;
		n = rem;
	}
	return m;
}
double calc4(std::string const& expr)
{
	std::string exp(expr);
	exp.erase(std::remove(exp.begin(), exp.end(), ' '), exp.end()); // Erase ' '
	if(exp.empty())
		return 0.0;
	std::string postfix;
	std::stack<char> operators;
	char top, c;
	for(std::string::const_iterator iter = exp.begin(); iter != exp.end(); ++iter){
		c = *iter;
		switch(c){
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
			while(!operators.empty() && !higher_priority(c, (top = operators.top()))){
				operators.pop();
				postfix += top;
				postfix += ' ';
			}
			operators.push(c);
			break;
		case ')':{
			while((top = operators.top()) != '('){
				operators.pop();
				postfix += top;
				postfix += ' ';
			}
			operators.pop();
			break;
				 }
		default:
			postfix +=  c;
			postfix += ' ';
			break;
		}
	}
	while(!operators.empty()){
		top = operators.top();
		operators.pop();
		postfix += top;
		postfix += ' ';
	}

	std::vector<std::string> tokens;
	boost::split(tokens, postfix, boost::is_any_of(" "));
	std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<std::string>(cout, ", "));
	cout << endl;

	double ret = 0.0;
	std::stack<std::string> tokenStack;
	for(std::vector<std::string>::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
	{
		if(iter->empty())
			continue;
		char c = iter->at(0);
		if(iter->size() == 1 && (c == '+' || c == '*' || c == '-' || c == '/')){
			double operand1 = std::strtod(tokenStack.top().c_str(), NULL);
			tokenStack.pop();
			double operand2 = std::strtod(tokenStack.top().c_str(), NULL);
			tokenStack.pop();
			char op = c;
			double result = 0.0;
			if('+' == op)
				result = operand1 + operand2;
			else if('-' == op)
				result = operand1 - operand2;
			else if('*' == op)
				result = operand1 * operand2;
			else if('/' == op)
				result = operand1 / operand2;

			char tmp[128];
			std::sprintf(tmp, "%f", result);
			tokenStack.push(tmp);

		}
		else
			tokenStack.push(*iter);
	}
	return std::strtod(tokenStack.top().c_str(), NULL);
}
void infix2postfix(char const* infix, char* postfix)
{
	if(!infix)
		return;
	std::stack<char> operators;
	char top;
	while(*infix){
		switch(*infix){
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
			while(!operators.empty() && !higher_priority(*infix, (top = operators.top()))){
				operators.pop();
				*postfix++ = top;
				*postfix++ = ' ';
			}
			operators.push(*infix);
			break;
		case ')':{
			while((top = operators.top()) != '('){
				operators.pop();
				*postfix++ = top;
				*postfix++ = ' ';
			}
			operators.pop();
			break;
				 }
		default:
			*postfix++ = *infix;
			*postfix++ = ' ';
			break;
		}
		++infix;
	}
	while(!operators.empty()){
		top = operators.top();
		operators.pop();
		*postfix++ = top;
		*postfix++ = ' ';
	}
}
bool higher_priority(char inputOpa, char stackOpb){
	return (inputOpa == '(' || 
		stackOpb == '(' || 
		((inputOpa == '*' || inputOpa == '/') && (stackOpb == '+' || stackOpb == '-'))
		);
	//switch(inputOpa){
	//		case '+':
	//		case '-':
	//			if(stackOpb == '+' || stackOpb == '-')
	//				return false;
	//			else if(stackOpb == '*' || stackOpb == '/')
	//				return false;
	//			else if(stackOpb == '(')
	//				return true;
	//		case '*':
	//		case '/':
	//			if(stackOpb == '+' || stackOpb == '-')
	//				return true;
	//			else if(stackOpb == '*' || stackOpb == '/')
	//				return false;
	//			else if(stackOpb == '(')
	//				return true;
	//		case '(':
	//			return true;
	//}
}
std::ostream& operator<<(std::ostream& ostr, const boost::container::list<int>& list)
{
	BOOST_FOREACH(int const& i, list) {
		ostr << " " << i;
	}
	ostr << std::endl;
	return ostr;
}
template<typename T> T Josephus(T* child, int n, int m)
{
	int kickOut = n - 1, curr = -1;	//要剔除的数量, 当前元素下标, 注意下标与数量的区别!
	while(true){
		for(int i = 0; i != m;){	//数m个数,数到下一个元素
			curr  = (curr + 1) % n; //curr在[0, n)之间循环(下标与数量之间的关系运算存在1的差)
			if(child[curr] != INT_MIN)
				++i;
		}
		if(kickOut == 1)
			break;
		cout << child[curr] << ", ";
		child[curr] = INT_MIN;		//标记为己剔出
		--kickOut;
	}
	cout << endl;
	return child[curr];
}
//////////////////////////////////////////////////////////////////////////
void maze(int m, int n){
	using namespace boost;
	std::vector<int>  rank (100);
	std::vector<int>  parent (100);
	boost::disjoint_sets<int*,int*> ds(&rank[0], &parent[0]);
	ds.union_set(0, 2);
}
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

//////////////////////////////////////////////////////////////////////////
int cut_rod(int const* p, int n){
	if( 0 == n)
		return 0;
	int q = INT_MIN;
	for(int i = 1; i <= n; ++i)
		q = std::max(p[i - 1], cut_rod(p, n - i));
	return q;
}
int bottom_up_cut_rod(int const* p, int n){
	std::map<int, int> r;	//长度与最大收益映射
	r[0] = 0;
	for(int j = 1; j <= n; ++j){
		int q = std::numeric_limits<int>::max();
		for(int i = 1; i <= j; ++i)
			q = std::max(q, p[i - 1] + r[j - i]);
		r[j] = q;
	}
	return r[n];
}
template<typename InsertIerator>
int bottom_up_cut_rod(int const* p, int n, InsertIerator it){
	std::map<int, int> r, s;	////长度与最大收益, 第一段钢条的切割长度映射, 
	r[0] = 0;
	for(int j = 1; j <= n; ++j){
		int q = INT_MIN;
		for(int i = 1; i <= j; ++i)
			if(q < p[i - 1] + r[j - i]){
				q = p[i - 1] + r[j - i];
				s[j] = i;
			}
		r[j] = q;
	}
	for(std::map<int, int>::const_iterator iter = s.begin(); iter != s.end(); ++iter)
		*it = iter->second;
	return r[n];
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
int compare_ints(const void* a, const void* b)   // comparison function
{
	int arg1 = *reinterpret_cast<const int*>(a);
	int arg2 = *reinterpret_cast<const int*>(b);
	if(arg1 < arg2) return -1;
	if(arg1 > arg2) return 1;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
my_vertex::my_vertex()
: color_(White), d_(0), f_(0), dist_(-1), p_(NULL)
{

}
clothes_vertex::clothes_vertex(std::string const& name)
: name_(name)
{

}

//////////////////////////////////////////////////////////////////////////
void breadth_first_search(dag_graph_t const& dag, int s)
{
	dag_graph_t g(dag);
	std::map<int, int> dist, parent;	//路径长, 父亲
	std::map<int, bool> known;			//是否己发现
	std::queue<int> found;				//己发现队列
	found.push(s);
	dist[s] = 0;
	parent[s] = -1;
	while(!found.empty()){
		int u = found.front();
		found.pop();
		dag_graph_t::mapped_type& lst = g[u];	//取其对应表
		typedef dag_graph_t::mapped_type::const_iterator const_iterator;
		for(const_iterator iter = lst.begin(); iter != lst.end(); ++iter){
			if(!known[*iter]){
				known[*iter] = true;
				dist[*iter] = dist[u] + 1;
				parent[*iter] = u;
				found.push(*iter);
			}
		}
		known[u] = true;
	}
}
template<typename Graph>
void breadth_first_search(Graph& g, typename Graph::vertex_descriptor const& s)
{
	using namespace boost;
	typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef graph_traits<Graph>::adjacency_iterator adjacency_iterator;

	std::queue<vertex_descriptor> Q;
	g[s].dist_ = 0;
	g[s].color_ = my_vertex::Gray;
	Q.push(s);
	while(!Q.empty()){
		vertex_descriptor u = Q.front(); Q.pop();
		adjacency_iterator aiter, aend;
		//遍历当前结点的邻接表
		for(tie(aiter, aend) = adjacent_vertices(u, g); aiter != aend; ++aiter){
			vertex_descriptor v = *aiter;
			if(g[v].color_ == my_vertex::White){
				g[v].color_ = my_vertex::Gray;
				g[v].p_ = &g[u];
				g[v].dist_ = g[u].dist_ + 1;
				Q.push(v);
			}
		}
		g[u].color_ = my_vertex::Black;
	}
}

template<typename Graph>
void depth_first_search(Graph& g)
{
	using namespace boost;
	typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef graph_traits<Graph>::vertex_iterator vertex_iterator;

	vertex_iterator viter, vend;
	//遍历每个顶点, 如果未发现就进入depth_first_visit
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter){
		if(g[*viter].color_ == my_vertex::White){
			depth_first_visit(g, *viter);
			cout << endl;
		}
	}
}
template<typename Graph, typename Vertex>
void print_path(Graph const& g, Vertex const& s, Vertex const& v)
{
	if(&s == &v)
		cout << s.dist_ << ", ";
	else if(!v.p_)
		cout << s.dist_ << "-->" << v.dist_ << " not exit" << endl;
	else{
		print_path(g, s, *v.p_);
		cout << v.dist_ << ", ";
	}
}

template<typename Graph>
void depth_first_visit(Graph& g, typename Graph::vertex_descriptor const& u)
{
	using namespace boost;
	typedef graph_traits<Graph>::adjacency_iterator adjacency_iterator;
	typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	static int time = 1;
	g[u].color_ = my_vertex::Gray;		//标记为己发现
	g[u].d_ = time++;					//发现时间
	cout << "(" << u << "/" << g[u].d_ << " ";

	adjacency_iterator aiter, aend;
	//遍历当前结点的邻接表
	for(tie(aiter, aend) = adjacent_vertices(u, g); aiter != aend; ++aiter){
		if(g[*aiter].color_ == my_vertex::White)
			depth_first_visit(g, *aiter);
	}
	g[u].color_ = my_vertex::Black;	//标记为己遍历
	g[u].f_ = time++;				//完成时间
	cout << " " << u << "/" << g[u].f_ << ")";

}

template<typename Graph>
void depth_first_search_non_recursive(Graph& g)
{
	using namespace boost;
	typedef graph_traits<Graph>::adjacency_iterator adjacency_iterator;
	typedef graph_traits<Graph>::vertex_iterator vertex_iterator;
	typedef graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	static int time = 1;
	vertex_iterator viter, vend;
	//遍历每个顶点
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter){
		vertex_descriptor u = *viter; 
		if(g[u].color_ == my_vertex::White){
			std::stack<vertex_descriptor> S;
			g[u].color_ = my_vertex::Gray;		//标记为己发现
			g[u].d_ = time++;					//发现时间
			cout << "(" << u << "/" << g[u].d_ << " ";
			S.push(u);
			while(!S.empty()){
				vertex_descriptor t = S.top();
				adjacency_iterator aiter, aend;
				//遍历当前结点的邻接表
				for(tie(aiter, aend) = adjacent_vertices(t, g); aiter != aend; ){
					vertex_descriptor v = *aiter; 
					if(g[v].color_ == my_vertex::White){
						g[v].color_ = my_vertex::Gray;		//标记为己发现
						g[v].d_ = time++;					//发现时间
						cout << "(" << v << "/" << g[v].d_ << " ";
						S.push(v);
						tie(aiter, aend) = adjacent_vertices(v, g);
						continue;
					}
					++aiter;
				}
				t = S.top();
				S.pop();
				g[t].color_ = my_vertex::Black;	//标记为己遍历
				g[t].f_ = time++;				//完成时间
				cout << " " << t << "/" << g[t].f_ << ")";
			}
			cout << endl;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
template<typename Graph, typename InsertIerator>
void topological_sort(Graph& g, InsertIerator iter)
{
	typedef boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
	typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	test::depth_first_search(g);

	vertex_iterator viter, vend; 
	std::map<int, vertex_descriptor> m;
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter)
		m[g[*viter].f_] = *viter;
	for(std::map<int, vertex_descriptor>::const_iterator it = m.begin(); it != m.end(); ++it)
		*iter = it->second;
}
//////////////////////////////////////////////////////////////////////////
namespace list{
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

}	//namespace list{
//////////////////////////////////////////////////////////////////////////
}	//namespace test
int test_main(int argc, char* argv[]){
	using namespace test;
	std::locale::global(std::locale(""));
	if(argc != 3)
		return -1;
	{
		//双链表
		using namespace test::list;
		cout << "双链表\n";
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
	{
		tstring str(argv[1]);
		std::wcout << str << _T("中字符各不相同为") << all_diff(str.begin(), str.end()) << std::endl;
		std::wcout << _T("reverse前") << argv[1];
		std::wcout << _T(", reverse后") <<my_reverse(argv[1])<< std::endl;
		std::wcout << argv[1] << _T(", ") << argv[2] << _T("元素是否相同:") << elements_same(argv[1], argv[2]) << std::endl;
		std::wcout << _T("替换") << argv[1] << _T("中的") << _T(" ") << _T("为") << _T("") ;
		std::wcout << replace_char_to_str(argv[1], _T(' '), _T("%20")) << std::endl;
		char src[] = "aabcccccaaa", dest[256] = "";
		compress_repeat_str(src, strlen(src), dest);
		std::wcout << _T("压缩前:") << src << std::endl << _T("压缩后:") << dest << std::endl;

	}
	{
		size_t const count = 256, start = 0, end = 255;
		int arr[count];
		for(size_t i = 0; i != count; ++i)
			arr[i] = roll_die(start, end);

	}
	{
		size_t const count = 256;
		int start = -255, end = 255;
		int arr[count], arr2[count];
		for(size_t i = 0; i != count; ++i)
			arr[i] = roll_die(start, end);

		std::copy(arr, arr + count, arr2);

		std::wcout << _T("归并排序前:") << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));

		merge_sort(arr, count);

		std::wcout << std::endl << _T("归并排序后:") << std::endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));
		std::cout << std::endl;

		std::wcout << std::endl << _T("std::sort()") << std::endl;
		std::sort(arr2, arr2 + count);
		std::copy(arr, arr + count, std::ostream_iterator<int>(std::cout, ", "));


		std::cout << std::endl;
	}
	{
		//最大子数组
		int arr[] = {-9, 9, -1, 7, -2, -8, -4, 0, -5, -2};
		size_t const count = sizeof(arr) / sizeof(arr[0]);
		cout << "最大子数组: " << endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(cout, ", "));
		cout << endl;
		int maxSum = 0;
		size_t low = 0, high = 0;
		maximun_subarray(arr, count, maxSum, low, high);
		std::printf("maxSum=%u, low=%u, high=%u\n", maxSum, low, high);
		maxSum = low = high = 0;
		maximun_subarray2(arr, count, maxSum, low, high);
		std::printf("maxSum=%u, low=%u, high=%u\n", maxSum, low, high);		


	}
	{
		//最大子数组2
		int arr[10];
		size_t const count = sizeof(arr) / sizeof(arr[0]);
		for(int i = 0; i != count; ++i){
			arr[i] = roll_die(-10, 10);
		}
		cout << "最大子数组: " << endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(cout, ", "));
		cout << endl;
		int maxSum = 0;
		size_t low = 0, high = 0;
		maximun_subarray(arr, count, maxSum, low, high);
		std::printf("maxSum=%u, low=%u, high=%u\n", maxSum, low, high);
		maxSum = low = high = 0;
		maximun_subarray2(arr, count, maxSum, low, high);
		std::printf("maxSum=%u, low=%u, high=%u\n", maxSum, low, high);		

	}
	{
		//二分搜索 
		size_t const count = 32;
		int arr[count];
		for(int i = 0; i != count; ++i){
			arr[i] = roll_die(0, 255);
		}
		std::sort(arr, arr + count);

		cout << "二分搜索: " << endl;
		std::copy(arr, arr + count, std::ostream_iterator<int>(cout, ", "));
		cout << endl;

		int i = roll_die(0, count - 1);
		int const& val = arr[i];
		std::printf("val = %d, index=%d\n", val, binary_search(arr, count, val));		
	}
	{
		//最大公因数
		int m = roll_die(2, 100), n = roll_die(2, 100);
		std::printf("最大公因数gcd(%d, %d) == %d\n", m, n, gcd(m, n));		

	}
	{
		//中缀表达式转后缀表达式
		size_t const count = 256;
		char infix[count] = "a+b*c+(d*e+f)*g+(h-i*j)";
		char postfix[count] = "";
		infix2postfix(infix, postfix);
		std::printf("中缀表达式转后缀表达式\n中缀表达式:%s\n后缀表达式:%s\n", infix, postfix);
		std::string expr = "8 + 5 * 2 + ( 1 + 5) * 2 + 1";
		std::printf("计算算术表达式的值, 限四则运算,可含括号:%s=%f\n", expr.c_str(), calc4(expr));
	}
	{
		//Josephus问题
		size_t const count = 5, n = roll_die(1, 2 * count);
		int childs[count];
		for(size_t i = 0; i != count; ++i)
			childs[i] = roll_die(1, 16);
		std::printf("Josephus问题: m=%u, n=%u\n", count, n);
		std::copy(childs, childs + count, std::ostream_iterator<int>(cout, ", "));
		cout << endl;
		std::printf("Josephus=%d\n", Josephus(childs, count, n));


	}
	{
		//汉诺塔问题
		int const n = 4;
		cout << "汉诺塔问题, 盘子数:" << n << endl;
		Hanoi('A', 'B', 'C', n);
	}
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
		cout << "二叉搜索树" << endl;
		ibtree.printTree(); cout << endl;
		cout << "是否平衡is_balanced:" << is_balanced(ibtree, ibtree.root()) << endl;
	}
	{
		//二叉堆
		std::vector<int> vecHeap = (boost::assign::list_of(8), 2, 5, 9, 6, 4, 7, 1), 
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
			cout << min << ", ";
		}
		cout << endl;
		
		//
		std::make_heap(v2.begin(), v2.end());
		std::sort_heap(v2.begin(), v2.end());

		cout << "堆排序:\nstd::make_heap/std::sort_heap:\n";
		std::copy(v2.begin(), v2.end(), std::ostream_iterator<int>(cout, ", "));
		cout << endl;

		build_max_heap(v3, v3.size());
		heap_sort(v3, v3.size());

		cout << "build_max_heap/heap_sort:\n";
		std::copy(v3.begin(), v3.end(), std::ostream_iterator<int>(cout, ", "));
		cout << endl;

		int v4[] = { -2, 99, 0, -743, 2, 3, 4 };
		size_t const size = sizeof(v4) / sizeof(v4[0]);
		int v5[size];
		std::copy(v4, v4 + size, v5);

		std::qsort(v4, size, sizeof(v4[0]), compare_ints);	
		quicksort(v5, 0, size - 1);	

		cout << "快速排序:\nstd::qsort:\n";
		std::copy(v4, v4 + size, std::ostream_iterator<int>(cout, ", "));
		cout << endl;

		cout << "快速排序:\nquicksort:\n";
		std::copy(v5, v5 + size, std::ostream_iterator<int>(cout, ", "));
		cout << endl;

	}
	{
		//动态规划:钢条切割问题
		int prices[] = {1, 5, 8, 9, 10, 17, 17, 20, 24, 30};
		int const length = sizeof(prices) / sizeof(prices[0]);
		std::vector<int> solutions;
		cout << "动态规划:钢条切割问题";
		std::copy(prices, prices + length, std::ostream_iterator<int>(cout, ", "));
		cout << endl << "最大收益(自顶向下递归实现):" << cut_rod(prices, length) << endl;
		cout << endl << "最大收益(自底向上):" << bottom_up_cut_rod(prices, length) << endl;
		cout << endl << "最大收益(自底向上, 并返回输出切割方案):" 
			<< bottom_up_cut_rod(prices, length, std::back_inserter(solutions)) << endl;
		for(size_t n = solutions.size(); n > 0; n = n - solutions[n - 1])
			cout << solutions[n - 1] << ", ";
		cout << endl;

	}
	{
		cout << "boost.property_map:" << endl;

		std::map<std::string, std::string> name2address;
		boost::associative_property_map< std::map<std::string, std::string> >
			address_map(name2address);

		name2address.insert(make_pair(std::string("Fred"), 
			std::string("710 West 13th Street")));
		name2address.insert(make_pair(std::string("Joe"), 
			std::string("710 West 13th Street")));

		foo(address_map);

		for (std::map<std::string, std::string>::iterator i = name2address.begin();
			i != name2address.end(); ++i)
			std::cout << i->first << ": " << i->second << "\n";

	}
	{
		//不相交集
		maze(10, 30);
	}
	{
		//广度优先搜索
		dag_graph_t dag;
		dag[1].push_back(2); dag[1].push_back(4);
		dag[3].push_back(1); dag[3].push_back(6);
		dag[2].push_back(4); dag[2].push_back(5);
		dag[4].push_back(3); dag[4].push_back(5); dag[4].push_back(6); dag[4].push_back(7);
		dag[5].push_back(7);
		dag[7].push_back(6);

		breadth_first_search(dag, 3);
	}
	{
		using namespace boost;
		// Select the graph type we wish to use
		typedef adjacency_list < vecS, vecS, undirectedS, my_vertex> graph_t;
		typedef graph_traits<graph_t>::vertices_size_type v_size_t;
		// Set up the my_vertex IDs and names
		int r = 0, s = 1, t = 2	, u = 3, v = 4, w = 5, x = 6, y = 7;
		v_size_t const N = 8;
		const char *name = "rstuvwxy";
		// Specify the edges in the graph
		typedef std::pair < int, int >E;
		E edge_array[] = { E(r, s), E(r, v), E(s, w), E(w, t),
			E(w, x), E(x, u), E(x, t), E(t, u), E(x, y), E(u, y)
		};
		// Create the graph object
		const int n_edges = sizeof(edge_array) / sizeof(E);
		graph_t g(edge_array, edge_array + n_edges, N), g2(g);

		// Typedefs
		typedef graph_traits < graph_t >::vertices_size_type Size;
		typedef Size* Iiter;

		// a vector to hold the discover time property for each my_vertex
		std::vector < Size > dtime(num_vertices(g));


		cout << "图的广度优先搜索, 广度优先树" << endl;
		cout << "(自己写的)" << endl;
		test::breadth_first_search(g2, vertex(s, g2));
		print_path(g2, g2[vertex(s, g2)], g2[vertex(y, g2)]);
		Size time = 0;
		bfs_time_visitor < Size * > vis(&dtime[0], time);
		cout << endl << "(boost.graph)" << endl;
		breadth_first_search(g, vertex(s, g), visitor(vis));

		// Use std::sort to order the vertices by their discover time
		std::vector<graph_traits<graph_t>::vertices_size_type > discover_order(N);
		integer_range < int >range(0, N);
		std::copy(range.begin(), range.end(), discover_order.begin());
		std::sort(discover_order.begin(), discover_order.end(),
			indirect_cmp < Iiter, std::less < Size > >(&dtime[0]));

		std::cout << "order of discovery: ";
		for (int i = 0; i < N; ++i)
			std::cout << name[discover_order[i]] << " ";
		std::cout << std::endl;

	}
	{
		//图的深度优先遍历
		using namespace boost;
		typedef adjacency_list<listS, vecS, directedS, my_vertex> graph_t;
		typedef std::pair< int, int> Edge;
		typedef graph_traits<graph_t>::vertices_size_type v_size_t;

		int u = 0, v = 1, w = 2, x = 3, y = 4, z = 5;
		Edge edges[] = { Edge(u, v), Edge(u, x), Edge(v, y), Edge(w, y), Edge(w, z), Edge(x, v), Edge(y, x)};
		size_t const n_edges = sizeof(edges) / sizeof(Edge);
		v_size_t const n_vertices = 6;

		graph_t g(edges, edges + n_edges, n_vertices), g2(g), g3(g);

		cout << "图的深度优先搜索" << endl;
		cout << "(自己写的, 递归形式)" << endl;
		test::depth_first_search(g);
		cout << "(自己写的, 非递归形式)" << endl;
		test::depth_first_search_non_recursive(g2);
		custom_dfs_visitor vis;
		cout << "(boost.graph)" << endl;
		depth_first_search(g3, visitor(vis));
	}
	{
		//深度优先搜索应用--拓扑排序
		using namespace boost;
		typedef adjacency_list<listS, vecS, directedS, clothes_vertex> graph_t;
		typedef graph_traits<graph_t>::vertices_size_type v_size_t;
		typedef graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
		typedef int clothes_t;
		typedef std::pair< clothes_t, clothes_t> dress_order_t;		//穿衣物的顺序

		clothes_t nei_ku = 0, ku_zi = 1, yao_dai = 2, cheng_yi = 3, 
					ling_dai = 4, ja_ke = 5, wa_zi = 6, xie = 7;
		dress_order_t edges[] = { dress_order_t(nei_ku, ku_zi), dress_order_t(ku_zi, yao_dai), 
								dress_order_t(cheng_yi, yao_dai), dress_order_t(cheng_yi, ling_dai), 
								dress_order_t(ling_dai, ja_ke), dress_order_t(yao_dai, ja_ke),  
								dress_order_t(nei_ku, xie), dress_order_t(ku_zi, xie), 
								dress_order_t(wa_zi, xie)
		};
		size_t const n_edges = sizeof(edges) / sizeof(edges[0]);
		v_size_t const n_vertices = 8;

		graph_t g(edges, edges + n_edges, n_vertices), g2(g);

		std::vector<vertex_descriptor> c, c2;
		test::topological_sort(g, std::back_inserter(c));
		cout << "A topological ordering(test::topological_sort): ";
		std::copy(c.rbegin(), c.rend(), std::ostream_iterator<vertex_descriptor>(cout, ", "));
		cout << endl;

		boost::topological_sort(g2, std::back_inserter(c2));
		cout << "A topological ordering(boost::topological_sort): ";
		std::copy(c2.rbegin(), c2.rend(), std::ostream_iterator<vertex_descriptor>(cout, ", "));
		cout << endl;
	}
	{
		//undirected_graph示例
		using namespace boost;
		undirected_graph<> g;
		undirected_graph<>::vertex_descriptor u = g.add_vertex(), 
			v = g.add_vertex(), w = g.add_vertex(), x = g.add_vertex();
		g.add_edge(u, v); g.add_edge(v, w); g.add_edge(w, x); g.add_edge(x, u);
		cout << "Degree of u: " << degree(u, g) << endl
			<< "顶点数" << g.num_vertices()
			<< "边数" << g.num_edges() << endl;

	}
	{
		//使用 BGL 进行图访问
		using namespace boost;

		typedef boost::adjacency_list< listS, vecS, directedS> mygraph;
		mygraph g;
		add_edge (0, 1, g);
		add_edge (0, 3, g);
		add_edge (1, 2, g);
		add_edge (2, 3, g);
		add_edge (7, 3, g);
		mygraph::vertex_iterator vertexIter, vertexEnd;
		mygraph::out_edge_iterator outEdgeIter, outEdgeEnd;
		mygraph::in_edge_iterator inEdgeIter, inEdgeEnd;
		tie(vertexIter, vertexEnd) = vertices(g);	//遍历结点
		//当要使用in_edges()时, 请使用bidirectionalS
		//tie(inEdgeIter, inEdgeEnd) = in_edges(*vertexIter, g);	//
		for(; vertexIter != vertexEnd; ++vertexIter){
			std::cout << "out-edges for " << *vertexIter << ": ";
			tie(outEdgeIter, outEdgeEnd) = out_edges(*vertexIter, g);	//遍历出度(出边)
			std::copy(outEdgeIter, outEdgeEnd, std::ostream_iterator<mygraph::edge_descriptor>(cout, ", "));
			//for(; outEdgeIter != outEdgeEnd; ++outEdgeIter)
			//	std::cout << *outEdgeIter << ", ";
			std::cout << endl;
		}
		mygraph::edge_iterator edgeIter, edgeEnd;
		tie(edgeIter, edgeEnd) = edges(g);
		for(; edgeIter != edgeEnd; ++edgeIter){	
			cout << "[" << source(*edgeIter, g) << *edgeIter << target(*edgeIter, g) << "]" << endl;
		}
	}
	{
		//最小生成树
		using namespace boost;
		typedef adjacency_list<listS, vecS, directedS, no_property, property<edge_weight_t, int> > graph_t;
		typedef graph_t::edge_descriptor Edge;
		graph_t g;
		add_edge (0, 1, 8, g);
		add_edge (0, 3, 18, g);
		add_edge (1, 2, 20, g);
		add_edge (2, 3, 2, g);
		add_edge (3, 1, 1, g);
		add_edge (1, 3, 7, g);
		std::list<Edge> spanning_tree;
		std::vector<graph_traits<graph_t>::vertex_descriptor> p(num_vertices(g));
		std::cout << "顶点个数:" << num_vertices(g) 
			 << "边数:" << num_edges(g)
			 << ", 最小生成树:" << endl;
		kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));
		prim_minimum_spanning_tree(g, &p[0]);
		std::copy(spanning_tree.begin(), spanning_tree.end(), std::ostream_iterator<Edge>(cout, ", "));
		std::cout << endl;
	}
	return EXIT_SUCCESS;
}	//test_main()

#endif
