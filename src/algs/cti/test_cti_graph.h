/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 图论
 * */
//////////////////////////////////////////////////////////////////////////
#ifndef TEST_CRACKING_THE_CODING_INTERVIEW_GRAPH
#define TEST_CRACKING_THE_CODING_INTERVIEW_GRAPH

#include <map>    /* std::map */
#include <string> /* std::string */
#include <vector> /* std::vector */
//图算法: 顶点
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

#endif /* TEST_CRACKING_THE_CODING_INTERVIEW_GRAPH */
