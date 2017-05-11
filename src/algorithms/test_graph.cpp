/*!
 * test graph, from book <Algorithms 4th Edition>_4
 * @author hongjun.liao <docici@126.com>, @date 2017//05/06
 *
 * visualization, see http://www.cs.usfca.edu/~galles/visualization/DFS.html
 */
///////////////////////////////////////////////////////////////////////////////////////////

#include "graph.h"	     /* graph */
#include "string_util.h" /* strnrchr */
#include "node_pool.h"	 /* node_new */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		 /* memmove */

/* tree_print.cpp */
extern void rbtree_draw_to_term(rb_tree const& tr);

static graph_node * graph_node_new(node_pool & p, int key);
static graph_bag * graph_bag_new(node_pool & p, graph_node * node, size_t v);

static void graph_node_del(rbtree_node * node);
static int graph_node_cmp(void const * na, void const * nb);
static char const * graph_node_c_str(void const * a, char * buf, size_t len);

static graph_bag * graph_bag_new(node_pool & p, graph_node * node, size_t v)
{
	if(p.i == POOL_N){
		fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
		exit(0);
	}
	auto RTR_POOL_N = (32 * 1024 / sizeof(graph_bag));

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (graph_bag * )calloc(RTR_POOL_N, sizeof(graph_bag));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto gb = &((graph_bag **)p.n)[p.i - 1][p.j++];
	gb->node = node;
	gb->v = (graph_node **)calloc(v, sizeof(graph_node *));
	gb->i = 0;

	return gb;
}


static graph_node * graph_node_new(node_pool & p, int key)
{
	if(p.i == POOL_N){
		fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
		exit(0);
	}
	auto RTR_POOL_N = (32 * 1024 / sizeof(graph_node));

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (graph_node * )calloc(RTR_POOL_N, sizeof(graph_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto gn = &((graph_node **)p.n)[p.i - 1][p.j++];
	gn->key = key;
	gn->weight = 0;

	return gn;
}

static void graph_node_del(rbtree_node * node)
{

}

static int graph_node_cmp(void const * na, void const * nb)
{
	auto gba = ((graph_bag *)na)->node, gbb = ((graph_bag *)nb)->node;
	if(!(gba && gbb))
		return 0;

	auto a = gba->key, b = gbb->key;

	return a < b? -1 : (a == b? 0 : 1);
}

static char const * graph_node_c_str(void const * data, char * buf, size_t len)
{
	auto n = (graph_bag * ) data;
	if(n && n->node)
		snprintf(buf, len, "%d", n->node->key);
	else
		snprintf(buf, len, "<null>");

//	fprintf(stdout, "%s: buf='%s'\n", __FUNCTION__, buf);
	return buf;
}

int test_graph_1_main(int argc, char ** argv)
{
	graph g;
	memset(&g, 0, sizeof(g));

	node_pool_init(g.tr.pool);
	g.tr.root = 0;
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;
	g.tr.node_data_free = 0;

	g.v = g.e = 0;

	node_pool_init(g.npool);
	node_pool_init(g.bpool);

	g.graph_node_new = graph_node_new;
	g.graph_bag_new = graph_bag_new;

	int N = 10;
	for(int i = 0; i < N; ++i){
		graph_add_vertex(g, i);
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

int graph_mst(graph const& g)
{
	fprintf(stdout, "%s: TODO: implement me!\n", __FUNCTION__);
	return -1;
}

int graph_init(graph & g, FILE * in)
{
	/* vertex and edge count */
	auto r = fscanf(in, "%zu", &g.v);
	if(r != 1) return -1;
	r = fscanf(in, "%zu", &g.e);
	if(r != 1) return -1;

	fprintf(stderr, "%s: vertex=%zu, edge=%zu\n", __FUNCTION__, g.v, g.e);

	for(size_t i = 0; i < g.v; ++i){
		auto r = graph_add_vertex(g, i);
		if(r != 0)
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
	memset(&g, 0, sizeof(g));

	node_pool_init(g.tr.pool);
	g.tr.root = 0;
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;
	g.tr.node_data_free = 0;

	g.v = g.e = 0;
	g.is_direct = true;

	node_pool_init(g.npool);
	node_pool_init(g.bpool);

	g.graph_node_new = graph_node_new;
	g.graph_bag_new = graph_bag_new;

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
	fprintf(stdout, "%s: graph_simple_dfs: [", __FUNCTION__);
	graph_simple_dfs(g, 0, stdout);
	fprintf(stdout, "]\n");

	/* graph mst */
	graph_mst(g);

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
