/*!
 * test graph, from book <Algorithms 4th Edition>_4
 * @author hongjun.liao <docici@126.com>, @date 2017//05/06
 *
 * visualization, see http://www.cs.usfca.edu/~galles/visualization/DFS.html
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include "rb_tree.h"	/* rb_tree */
#include "node_pool.h"	/* node_new */
#include <stdio.h>
#include <stdlib.h>

/* tree_print.cpp */
extern void rbtree_draw_to_term(rb_tree const& tr);

struct graph_node;
struct graph;

static graph_node * graph_node_new(node_pool & p, int key);
static void graph_node_del(rbtree_node * node);
static int graph_node_cmp(void const * gna, void const * gnb);
static char const * graph_node_c_str(void const * a, char * buf, size_t len);

struct graph_node {
	rbtree_node * node;
	int key;
};
struct graph {
	rb_tree tr;
};

static graph_node * graph_node_new(node_pool & p, int key)
{
	if(p.i == 32){
		fprintf(stderr, "%s: outof memory\n", __FUNCTION__);
		return 0;
	}
	auto RTR_POOL_N = (32 * 1024 / sizeof(graph_node));

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (graph_node * )calloc(RTR_POOL_N, sizeof(graph_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto ret = &((graph_node **)p.n)[p.i - 1][p.j++];
	ret->key = key;

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

	return buf;
}

static int graph_init(graph & g, int v)
{
	g.tr.root = 0;
	node_pool_init(g.tr.pool);
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;

	node_pool pool;
	node_pool_init(pool);
	for(int i = 0; i < v; ++i){
		rbtree_insert(g.tr, graph_node_new(pool, i));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
int graph_add_edge(graph & g, int v, int w)
{
	auto nodev = rbtree_search(g.tr, &v);
	auto nodew = rbtree_search(g.tr, &w);
	if(!nodev || !nodew)
		return -1;

	auto adj = (rbtree_node *)nodev->data;
}

/////////////////////////////////////////////////////////////////////////////////////
//main
int test_graph_main(int argc, char ** argv)
{
	graph g;
	graph_init(g, 20);

	rbtree_draw_to_term(g.tr);
	return 0;
}
