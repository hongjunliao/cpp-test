/*!
 * 数据结构与算法分析(C++描述)_第３版
 * @author hongjun.liao <docici@126.com>, @date 2017//05/05/18
 * 图论
 * */
//////////////////////////////////////////////////////////////////////////
#include "test_cti_graph.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>            /* std::cout, cin, std::endl */
#include <queue>               /* std::queue */
#include <stack>               /* std::stack */
#include <boost/graph/adjacency_list.hpp>
#include "boost/graph/prim_minimum_spanning_tree.hpp"	//prim算法, 最小生成树
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/undirected_graph.hpp>             /* boost::undirected_graph */
#include "boost/graph/kruskal_min_spanning_tree.hpp"	//最小生成树

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
		std::cout << std::endl;
	}
};

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
	typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;

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
	typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
	typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;

	vertex_iterator viter, vend;
	//遍历每个顶点, 如果未发现就进入depth_first_visit
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter){
		if(g[*viter].color_ == my_vertex::White){
			depth_first_visit(g, *viter);
			std::cout << std::endl;
		}
	}
}
template<typename Graph, typename Vertex>
void print_path(Graph const& g, Vertex const& s, Vertex const& v)
{
	if(&s == &v)
		std::cout << s.dist_ << ", ";
	else if(!v.p_)
		std::cout << s.dist_ << "-->" << v.dist_ << " not exit" << std::endl;
	else{
		print_path(g, s, *v.p_);
		std::cout << v.dist_ << ", ";
	}
}

template<typename Graph>
void depth_first_visit(Graph& g, typename Graph::vertex_descriptor const& u)
{
	using namespace boost;
	typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;
	typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	static int time = 1;
	g[u].color_ = my_vertex::Gray;		//标记为己发现
	g[u].d_ = time++;					//发现时间
	std::cout << "(" << u << "/" << g[u].d_ << " ";

	adjacency_iterator aiter, aend;
	//遍历当前结点的邻接表
	for(tie(aiter, aend) = adjacent_vertices(u, g); aiter != aend; ++aiter){
		if(g[*aiter].color_ == my_vertex::White)
			depth_first_visit(g, *aiter);
	}
	g[u].color_ = my_vertex::Black;	//标记为己遍历
	g[u].f_ = time++;				//完成时间
	std::cout << " " << u << "/" << g[u].f_ << ")";

}

template<typename Graph>
void depth_first_search_non_recursive(Graph& g)
{
	using namespace boost;
	typedef typename graph_traits<Graph>::adjacency_iterator adjacency_iterator;
	typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
	typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	static int time = 1;
	vertex_iterator viter, vend;
	//遍历每个顶点
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter){
		vertex_descriptor u = *viter;
		if(g[u].color_ == my_vertex::White){
			std::stack<vertex_descriptor> S;
			g[u].color_ = my_vertex::Gray;		//标记为己发现
			g[u].d_ = time++;					//发现时间
			std::cout << "(" << u << "/" << g[u].d_ << " ";
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
						std::cout << "(" << v << "/" << g[v].d_ << " ";
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
				std::cout << " " << t << "/" << g[t].f_ << ")";
			}
			std::cout << std::endl;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
template<typename Graph, typename InsertIerator>
void topological_sort(Graph& g, InsertIerator iter)
{
	typedef typename boost::graph_traits<Graph>::vertex_iterator vertex_iterator;
	typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

	depth_first_search(g);

	vertex_iterator viter, vend;
	std::map<int, vertex_descriptor> m;
	for(tie(viter, vend) = vertices(g); viter != vend; ++viter)
		m[g[*viter].f_] = *viter;
	for(auto it = m.begin(); it != m.end(); ++it)
		*iter = it->second;
}

int test_cti_graph_main(int argc, char* argv[])
{
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


		std::cout << "图的广度优先搜索, 广度优先树" << std::endl;
		std::cout << "(自己写的)" << std::endl;
		breadth_first_search(g2, vertex(s, g2));
		print_path(g2, g2[vertex(s, g2)], g2[vertex(y, g2)]);
		Size time = 0;
		bfs_time_visitor < Size * > vis(&dtime[0], time);
		std::cout << std::endl << "(boost.graph)" << std::endl;
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

		std::cout << "图的深度优先搜索" << std::endl;
		std::cout << "(自己写的, 递归形式)" << std::endl;
		depth_first_search(g);
		std::cout << "(自己写的, 非递归形式)" << std::endl;
		depth_first_search_non_recursive(g2);
		custom_dfs_visitor vis;
		std::cout << "(boost.graph)" << std::endl;
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
		topological_sort(g, std::back_inserter(c));
		std::cout << "A topological ordering(topological_sort): ";
		std::copy(c.rbegin(), c.rend(), std::ostream_iterator<vertex_descriptor>(std::cout, ", "));
		std::cout << std::endl;

		topological_sort(g2, std::back_inserter(c2));
		std::cout << "A topological ordering(boost::topological_sort): ";
		std::copy(c2.rbegin(), c2.rend(), std::ostream_iterator<vertex_descriptor>(std::cout, ", "));
		std::cout << std::endl;
	}
	{
		//undirected_graph示例
		using namespace boost;
		undirected_graph<> g;
		undirected_graph<>::vertex_descriptor u = g.add_vertex(),
			v = g.add_vertex(), w = g.add_vertex(), x = g.add_vertex();
		g.add_edge(u, v); g.add_edge(v, w); g.add_edge(w, x); g.add_edge(x, u);
		std::cout << "Degree of u: " << degree(u, g) << std::endl
			<< "顶点数" << g.num_vertices()
			<< "边数" << g.num_edges() << std::endl;

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
			std::copy(outEdgeIter, outEdgeEnd, std::ostream_iterator<mygraph::edge_descriptor>(std::cout, ", "));
			//for(; outEdgeIter != outEdgeEnd; ++outEdgeIter)
			//	std::cout << *outEdgeIter << ", ";
			std::cout << std::endl;
		}
		mygraph::edge_iterator edgeIter, edgeEnd;
		tie(edgeIter, edgeEnd) = edges(g);
		for(; edgeIter != edgeEnd; ++edgeIter){
			std::cout << "[" << source(*edgeIter, g) << *edgeIter << target(*edgeIter, g) << "]" << std::endl;
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
			 << ", 最小生成树:" << std::endl;
		kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));
		prim_minimum_spanning_tree(g, &p[0]);
		std::copy(spanning_tree.begin(), spanning_tree.end(), std::ostream_iterator<Edge>(std::cout, ", "));
		std::cout << std::endl;
	}

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
