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
#include <math.h>        /* log2 */
#include <string.h>		 /* memmove */
#include <sys/stat.h>	 /* fstat */
#include <unistd.h>      /* sleep */

/* tdarr.cpp */
extern int test_tdarr_main(int argc, char ** argv);
/* tree_print.cpp */
extern void rbtree_draw_to_term(rb_tree const& tr);

static graph_node * graph_node_new(node_pool & p, int key);
static graph_bag * graph_bag_new(node_pool & p, graph_node * node, size_t v, bool weight);

static void graph_node_del(rbtree_node * node);
static int graph_node_cmp(void const * na, void const * nb);
static char const * graph_node_c_str(void const * a, char * buf, size_t len);
static void graph_bag_realloc(graph_bag * gb, bool weight);

#define BAG_N  (1024 * 1024 * 8 / sizeof(graph_bag))
#define NODE_N (1024 * 1024 * 4 / sizeof(graph_node))

static graph_bag * graph_bag_new(node_pool & p, graph_node * node, size_t v, bool weight)
{
	if(!p.n[0]){
		p.n[0] = (graph_bag * )malloc(BAG_N * sizeof(graph_bag));
		if(!p.n[0]){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
	}

	if(p.i == p.N - 1){
		auto NN = (size_t)log2(p.N);
		p.N += NN < 8? 8 : NN;
		p.n = (void **)realloc(p.n, p.N * sizeof(void *));
		if(!p.n){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
	}

	if(p.j == BAG_N - 1){
		++p.i;
		p.n[p.i] = (graph_bag * )malloc(BAG_N * sizeof(graph_bag));
		if(!p.n[p.i]){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
		p.j = 0;
	}

	auto gb = &((graph_bag **)p.n)[p.i][p.j++];
	gb->node = node;
	gb->w = 0;

	auto I = (size_t)log2((double)v);
	gb->I = I < 8? 8 : I;
	gb->v = (graph_node **)malloc(gb->I * sizeof(graph_node *));
	if(weight)
		gb->w = (double * )malloc(gb->I * sizeof(double));

	return gb;
}

static graph_node * graph_node_new(node_pool & p, int key)
{
	if(!p.n[0]){
		p.n[0] = (graph_node * )malloc(NODE_N * sizeof(graph_node));
		if(!p.n[0]){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
	}

	if(p.i == p.N - 1){
		auto NN = (size_t)log2(p.N);
		p.N += NN < 8? 8 : NN;
		p.n = (void **)realloc(p.n, p.N * sizeof(void *));
		if(!p.n){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
	}

	if(p.j == NODE_N - 1){
		++p.i;
		p.n[p.i] = (graph_node * )malloc(NODE_N * sizeof(graph_node));
		if(!p.n[p.i]){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
		p.j = 0;
	}

	auto gn = &((graph_node **)p.n)[p.i][p.j++];
	gn->key = key;

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

static void graph_bag_realloc(graph_bag * gb, bool weight)
{
	if(!gb || gb->i < gb->I - 1)
		return;

	auto II = (size_t)log2((double)gb->I);
	gb->I += II < 8? 8 : II;
	gb->v = (graph_node **)realloc(gb->v, gb->I * sizeof(graph_node *));
	if(weight)
		gb->w = (double * )realloc(gb->w, gb->I * sizeof(double));
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

/* undirect_unweight_graph
 * shell cmd: ./cpp-test-main graph ~/ws/cpp-test/src/algorithms/rc/tinyG.txt
 * */
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
	g.direct = false;
	g.weight = false;

	node_pool_init(g.npool);
	node_pool_init(g.bpool);

	g.graph_node_new = graph_node_new;
	g.graph_bag_new = graph_bag_new;
	g.graph_bag_realloc = graph_bag_realloc;

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
	size_t adj_sz;
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

/* undirect_weight_graph
 * shell cmd: ./cpp-test-main graph ~/ws/cpp-test/src/algorithms/rc/tinyEWG.txt
 * */
int test_wgraph_main(int argc, char ** argv)
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
	auto fno = fileno(in);
	struct stat fs;
	if(fstat(fno, &fs) < 0){
		fprintf(stderr, "%s: fstat() failed\n", __FUNCTION__);
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
	g.direct = false;
	g.weight = true;

	node_pool_init(g.npool);
	node_pool_init(g.bpool);

	g.graph_node_new = graph_node_new;
	g.graph_bag_new = graph_bag_new;
	g.graph_bag_realloc = graph_bag_realloc;

	auto f1 = [](size_t total, size_t n) {
		if((n == total || (n <= 10000000 && n % 100000 == 0) ||
				(n <= 1000000 && n % 10000 == 0) || (n <= 100000 && n % 1000 == 0)))
			fprintf(stdout, "\rwgraph_init: processed %zu/%zu %5.1f %% vertex", n, total,
					n == total? 100.0 : n * 1.0 / total * 100.0);
		fflush(stdout);
	};
	auto f2 = [](size_t total, size_t n) { fprintf(stdout, "\n"); };
	auto f3 = [](size_t total, size_t n) {
		if((n == total || (n <= 10000000 && n % 100000 == 0) ||
				(n <= 1000000 && n % 10000 == 0) || (n <= 100000 && n % 1000 == 0)))
			fprintf(stdout, "\rwgraph_init: processed %zu/%zu %5.1f %% edges", n, total,
					n == total? 100.0 : n * 1.0 / total * 100.0);
		fflush(stdout);
	};
	auto f4 = [](size_t total, size_t n) { fprintf(stdout, "\n"); };

	auto result = wgraph_init(g, in, f1, f2, f3, f4);
	if(result != 0){
		fprintf(stdout, "%s: graph_init failed!\n", __FUNCTION__);
		return -1;
	}
	/* graph to string */
	size_t len;
	graph_c_str(g, 0, len);
	if(len == 0){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		return -1;
	}
	auto graphbuf = (char *)malloc(len);
	auto p = graph_c_str(g, graphbuf, len);
	if(!p){
		fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
		free(graphbuf);
		return -1;
	}
	fprintf(stdout, "%s: graph(%zu, %zu)=\n%s\n", __FUNCTION__, g.v, g.e, graphbuf);
	free(graphbuf);
	return 0;
}

static int test_dgraph_reverse_main(int argc, char ** argv)
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
	g.direct = true;
	g.weight = false;

	node_pool_init(g.npool);
	node_pool_init(g.bpool);

	g.graph_node_new = graph_node_new;
	g.graph_bag_new = graph_bag_new;
	g.graph_bag_realloc = graph_bag_realloc;

	auto result = graph_init(g, stdin);
	if(result != 0){
		fprintf(stderr, "%s: graph_init failed!\n", __FUNCTION__);
		return -1;
	}
	/* graph to string */
	{
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
	}

	{
		auto && rg = dgraph_reverse_copy(g);
		size_t len;
		graph_c_str(rg, 0, len);
		if(len == 0){
			fprintf(stdout, "%s: graph_c_str failed(get length)!\n", __FUNCTION__);
			return -1;
		}
		char graphbuf[len];
		auto p = graph_c_str(rg, graphbuf, len);
		if(!p){
			fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
			return -1;
		}
		fprintf(stdout, "%s: reverse(copy) graph=\n%s\n", __FUNCTION__, graphbuf);
	}
	{
		dgraph_reverse(g);
		size_t len;
		graph_c_str(g, 0, len);
		if(len == 0){
			fprintf(stdout, "%s: graph_c_str failed(get length)!\n", __FUNCTION__);
			return -1;
		}
		char graphbuf[len];
		auto p = graph_c_str(g, graphbuf, len);
		if(!p){
			fprintf(stdout, "%s: graph_c_str failed!\n", __FUNCTION__);
			return -1;
		}
		fprintf(stdout, "%s: reverse graph=\n%s\n", __FUNCTION__, graphbuf);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//main
int test_graph_main(int argc, char ** argv)
{
//	test_graph_1_main(argc, argv);
//	test_graph_init_from_file_main(argc, argv);
//	test_tdarr_main(argc, argv);
//	test_wgraph_main(argc, argv);
	test_dgraph_reverse_main(argc, argv);

//	node_pool pool;
//	node_pool_init(pool);
//	for(size_t i = 0; node_new(pool, 0); ++i){
//		if( i > 0 && i % 100000 == 0){
//			printf("%s: node_new, node=%zu\n", __FUNCTION__, i + 1);
//			sleep(1);
//		}
//	}

//	node_pool pool;
//	node_pool_init(pool);
//	for(size_t i = 0; graph_bag_new(pool, 0, 1000000, true); ++i){
//		if( i % 100000 == 0){
//			printf("%s: graph_bag_new, i=%zu\n", __FUNCTION__, i);
//			sleep(1);
//		}
//	}

//	node_pool pool;
//	node_pool_init(pool);
//	for(size_t i = 0; graph_node_new(pool, i); ++i){
//		if( i % 100000 == 0){
//			printf("%s: graph_node_new, i=%zu\n", __FUNCTION__, i);
//			sleep(1);
//		}
//	}

	return 0;
}
