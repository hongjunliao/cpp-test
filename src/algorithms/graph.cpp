/*!
 * test graph, from book <Algorithms 4th Edition>_4
 * @author hongjun.liao <docici@126.com>, @date 2017//05/06
 *
 * visualization, see http://www.cs.usfca.edu/~galles/visualization/DFS.html
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include "rb_tree.h"	 /* rb_tree */
#include "string_util.h" /* strnrchr */
#include "node_pool.h"	 /* node_new */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		 /* memmove */
struct graph_node;
struct graph;

/* tree_print.cpp */
extern void rbtree_draw_to_term(rb_tree const& tr);

static graph_node * graph_node_new(node_pool & p, int key, int n);
static void graph_node_del(rbtree_node * node);
static int graph_node_cmp(void const * gna, void const * gnb);
static char const * graph_node_c_str(void const * a, char * buf, size_t len);

struct graph_node {
	int key;

	int * v;
	int i;
};

struct graph {
	rb_tree tr;
	node_pool pool;    /* for graph_node */
	size_t v;		   /* vertex */
	size_t e;          /* edges */

	bool * dfs_marked; /* for dfs */
};

static graph_node * graph_node_new(node_pool & p, int key, int n)
{
	if(p.i == POOL_N){
		fprintf(stderr, "%s: outof memory\n", __FUNCTION__);
		exit(0);
	}
	auto RTR_POOL_N = (32 * 1024 / sizeof(graph_node));

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (graph_node * )calloc(RTR_POOL_N, sizeof(graph_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto ret = &((graph_node **)p.n)[p.i - 1][p.j++];
	ret->key = key;
	ret->v = (int *)malloc(n * sizeof(int));
	ret->i = 0;

	return ret;
}


static void graph_node_del(rbtree_node * node)
{

}

static int graph_node_cmp(void const * gna, void const * gnb)
{
	auto a = ((graph_node *)gna)->key, b = ((graph_node *)gnb)->key;
	return a < b? -1 : (a == b? 0 : 1);
}

static char const * graph_node_c_str(void const * data, char * buf, size_t len)
{
	auto node = (graph_node * ) data;
	if(node)
		snprintf(buf, len, "%d", node->key);
	else
		snprintf(buf, len, "<null>");

//	fprintf(stdout, "%s: buf='%s'\n", __FUNCTION__, buf);
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////
rbtree_node * graph_search(graph const& g, int key)
{
	return rbtree_search(g.tr, &key);
}

int graph_add_edge(graph & g, int v, int w)
{
	auto nodev = graph_search(g, v);
	auto nodew = graph_search(g, w);
	if(!(nodev && nodew && nodev->data && nodew->data))
		return -1;

	auto gnode = (graph_node *)nodev->data;

	gnode->v[gnode->i++] = ((graph_node * )nodew->data)->key;
	return 0;
}

int const * graph_adj(graph const& g, int v, int & sz)
{
	auto nodev = graph_search(g, v);
	if(!(nodev && nodev->data)){
		sz = 0;
		return 0;
	}
	auto gnode = (graph_node *)nodev->data;
	sz = gnode->i;
	return gnode->v;
}

static void do_graph_simple_dfs(graph & g, int v)
{
	g.dfs_marked[v] = true;
	fprintf(stdout, "%s: found %d\n", __FUNCTION__, v);

	int sz;
	auto adj = graph_adj(g, v, sz);
	for(int i = 0; i < sz; ++i){
		if(!g.dfs_marked[adj[i]])
			do_graph_simple_dfs(g, adj[i]);
	}
}
/* simple depth first search for graph */
void graph_simple_dfs(graph & g, int v)
{
	g.dfs_marked = (bool *)realloc(g.dfs_marked, g.v * sizeof(bool));
	memset(g.dfs_marked, 0, g.v * sizeof(bool));

	return do_graph_simple_dfs(g, v);
}
/* graph to string, output format:
 * vertex0: vertex0, vertex1, vertex, ...
 * vertex1: vertex0, vertex1, ...
 * ...
 *
 * sample:
 *  0: 5, 1, 2, 6,
 *	4: 3,
 *	5: 4, 3,
 *	6: 4,
 *	7: 8,
 *	9: 12, 10, 11,
 *	11: 12,
 *
 * @return:
 * if actual length of graph > @param len or error ocurred, then return NULL
 * @param len set to miminal length of graph or set to 0 if error
 * */
char * graph_c_str(graph const& g, char * buf, size_t& len)
{
	if(!buf)
		len = 0;

	void * vers[g.v];
	int vlen;
	rbtree_inorder_walk(g.tr, vers, vlen);

	char tmp[128];
	size_t n = 0;
	for(int i = 0; i < vlen; ++i){
		auto gnode = (graph_node *)vers[i];
		if(!gnode || gnode->i == 0)
			continue;

		auto r = buf && len > n?
				snprintf(buf + n, len - n, "\t%d: ", gnode->key):
				snprintf(tmp, 128, "\t%d: ", gnode->key);

		if(r < 0)
			return 0;

		n += r;

		for(auto j = 0; j < gnode->i; ++j){
			auto r2 = buf && len > n?
					snprintf(buf + n, len - n, "%d, ", gnode->v[j]):
					snprintf(tmp, 128, "%d, ", gnode->v[j]);

			if(r2 < 0)
				return 0;

			n += r2;
		}
		if(buf && len > n)
			buf[n] = '\n';
		++n;
	}
	if(!buf || len < n){
		len = n;
		return 0;
	}
	if(buf && len >= n)
		buf[n - 1] = '\0';
	fprintf(stdout, "%s: vertex='%d', strlen=%zu\n", __FUNCTION__, vlen, n);

	return buf;
}

/* init graph @param g from file @param f, file @param f has the following format:
 * vertex_total         <---total vertex
 * edge_total           <---total edges
 * vertex1 vertex2      <---one edge
 * vertex1 vertex2      <---another edge
 * ...                  <---...(other edges)
 *
 * sample('\n' means newline):
 * 13\n13\n0 5\n4 3\n0 1\n9 12\n6 4\n5 4\n0 2\n11 12\n9 10\n0 6\n7 8\n9 11\n5 3
 * */
static int graph_init(graph & g, FILE * in)
{
	memset(&g, 0, sizeof(g));

	node_pool_init(g.pool);
	node_pool_init(g.tr.pool);
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;

	/* vertex and edge count */
	auto r = fscanf(in, "%zu", &g.v);
	if(r != 1) return -1;
	r = fscanf(in, "%zu", &g.e);
	if(r != 1) return -1;

	fprintf(stderr, "%s: vertex=%zu, edge=%zu\n", __FUNCTION__, g.v, g.e);

	for(size_t i = 0; i < g.v; ++i){
		auto node = rbtree_insert(g.tr, graph_node_new(g.pool, i, g.v));
		if(!node)
			return -1;
	}

	/* load edge data */
	int w, x;
	while((r = fscanf(in, "%d%d",  &w, &x)) != EOF){
		if(r != 2) continue;

		if(graph_add_edge(g, w, x) != 0){
			fprintf(stderr, "%s: graph_add_edge for %d-%d failed, skip\n", __FUNCTION__, w, x);
		}
	}
	return 0;
}

int test_graph_1_main(int argc, char ** argv)
{
	graph g;
	node_pool_init(g.pool);
	node_pool_init(g.tr.pool);
	g.v = g.e = 0;
	g.tr.root = 0;
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;
	g.tr.node_data_free = 0;

	int N = 10;
	for(int i = 0; i < N; ++i){
		rbtree_insert(g.tr, graph_node_new(g.pool, i, N));
	}

	rbtree_draw_to_term(g.tr);

	char buf[32];
	fprintf(stdout, "%s: root=%s, [", __FUNCTION__,
			g.tr.node_c_str(g.tr.root->data, buf, sizeof(buf)));
	rbtree_inorder_walk(g.tr);
	fprintf(stdout, "]\n");

	if(graph_add_edge(g, 0, 1) != 0)
		fprintf(stdout, "%s: graph_add_edge failed for %d-%d\n", __FUNCTION__, 0, 1);
	graph_add_edge(g, 7, 3);
	graph_add_edge(g, 7, 6);

	size_t len;
	graph_c_str(g, 0, len);
	if(len == 0){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		return -1;
	}

	char graphbuf[len];
	auto p = graph_c_str(g, graphbuf, len);
	if(!p){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		return -1;
	}
	fprintf(stdout, "%s: graph=\n%s\n", __FUNCTION__, graphbuf);

	return 0;
}

int test_graph_init_from_file_main(int argc, char ** argv)
{
	/* graph init from file  */
	if(argc < 2){
		fprintf(stdout, "usage: %s <graph_file>\n", __FUNCTION__);
		return -1;
	}
	auto f = argv[1];
	auto in = fopen(f, "r");
	if(!in){
		fprintf(stdout, "%s: fopen '%s' failed\n", __FUNCTION__, f);
		return -1;
	}

	graph g;
	auto result = graph_init(g, in);
	if(result != 0){
		fprintf(stdout, "%s: graph_init failed!\n", __FUNCTION__);
		return -1;
	}

	/* graph print internal rbtree */
	rbtree_draw_to_term(g.tr);

	char buf[32];
	fprintf(stdout, "%s: root=%s, [", __FUNCTION__,
			g.tr.node_c_str(g.tr.root->data, buf, sizeof(buf)));
	rbtree_inorder_walk(g.tr);
	fprintf(stdout, "]\n");

	/* graph to string */
	size_t len;
	graph_c_str(g, 0, len);
	if(len == 0){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		return -1;
	}
	char graphbuf[len];
	auto p = graph_c_str(g, graphbuf, len);
	if(!p){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		return -1;
	}
	fprintf(stdout, "%s: graph=\n%s\n", __FUNCTION__, graphbuf);

	/* graph adj */
	int adj_sz;
	if(!graph_adj(g, -1, adj_sz))
		fprintf(stdout, "%s: graph_adj NOT found for %d\n", __FUNCTION__, -1);
	auto adj = graph_adj(g, 9, adj_sz);
	if(!adj){
		fprintf(stdout, "%s: graph_adj NOT found for %d\n", __FUNCTION__, 9);
	}

	/* graph dfs */
	graph_simple_dfs(g, 0);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
//main
int test_graph_main(int argc, char ** argv)
{
//	test_graph_1_main(argc, argv);
	test_graph_init_from_file_main(argc, argv);
	return 0;
}
