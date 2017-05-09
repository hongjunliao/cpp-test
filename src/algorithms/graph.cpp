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
#include <string.h>		/* memmove */
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
	node_pool pool;
};

static graph_node * graph_node_new(node_pool & p, int key, int n)
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

char * graph_c_str(graph const& g, char * buf, size_t len)
{
	void * vers[1024];
	int vlen;
	rbtree_inorder_walk(g.tr, vers, vlen);

	size_t n = 0;
	for(int i = 0; i < vlen; ++i){

		auto gnode = (graph_node *)vers[i];
		if(!gnode)
			continue;

		auto r = snprintf(buf + n, len - n, "\t%d: ", gnode->key);
		if(r < 0){
			buf[0] = '\0';
			return buf;
		}
		n += r;

		for(auto j = 0; j < gnode->i; ++j){
			int r2 = 0;
			r2 = snprintf(buf + n, len - n, "%d, ", gnode->v[j]);
			if(r2 < 0){
				buf[0] = '\0';
				return buf;
			}
			n += r2;
		}

		buf[n++] = '\n';
	}
	buf[n - 1] = '\0';
	fprintf(stdout, "%s: vers='%d', n=%zu\n", __FUNCTION__, vlen, n);

	return buf;
}

static char const * array_rchr(char const * buf, int sz, char ch = '\n')
{
	for(auto const * p = buf + sz - 1; p != buf - 1; --p) {
		if(*p == ch)
			return p;
	}
	return NULL;
}

static int do_graph_init(char const * buf, size_t len)
{
	fprintf(stdout, "%s: TODO: implement me!!!\n", __FUNCTION__);
	return -1;
}

/* init graph @param g from file @param f, file @param f has the following format:
 * vertex_total         <---total vertex
 * edge_total           <---total edges
 * vertex1 vertex2      <---one edge
 * vertex1 vertex2      <---another edge
 * ...                  <---other edges
 *
 * sample('\n' is newline):
 * 13\n\n13\n0 5\n4 3\n0 1\n9 12\n6 4\n5 4\n0 2\n11 12\n9 10\n0 6\n7 8\n9 11\n5 3
 * */
static int graph_init(graph & g, FILE * in)
{
	size_t BUF = 1024 * 4;
	auto buf = (char *)malloc(sizeof(char) * (BUF));
	char * p = buf;
	size_t count = 0;
	for(;; ++count){
		auto sz = fread(p, sizeof(char), BUF, in);
		if(ferror(in)){
			fprintf(stderr, "%s: fread failed, exit\n", __FUNCTION__);
			return -1;
		}
		if(sz == 0)
			break;
		auto end = array_rchr(buf, sz + p - buf);
		if(!end)
			continue;	/* no '\n' and no error, a huge-long line? */
		++end;	/* skip '\n' */
		auto result = do_graph_init(buf, end - buf);
		if(result != 0){
			fprintf(stderr, "%s: do_graph_init faield\n", __FUNCTION__);
			return -1;
		}
		auto len = p + sz - end;
		memmove(buf, end, len);
		p = buf + len;
	}
	free(buf);
	return 0;
}

int test_graph_1_main(int argc, char ** argv)
{
	graph g;
	node_pool_init(g.pool);
	node_pool_init(g.tr.pool);
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

	char graphbuf[1024];
	graph_c_str(g, graphbuf, sizeof(graphbuf));
	fprintf(stdout, "%s: graph=\n%s\n", __FUNCTION__, graphbuf);

	return 0;
}

int test_graph_init_from_file_main(int argc, char ** argv)
{
	graph g;
	node_pool_init(g.pool);
	node_pool_init(g.tr.pool);
	g.tr.root = 0;
	g.tr.node_new = node_new;
	g.tr.node_cmp = graph_node_cmp;
	g.tr.node_del = graph_node_del;
	g.tr.node_c_str = graph_node_c_str;
	g.tr.node_data_free = 0;

	auto result = graph_init(g, stdin);
	if(result != 0){
		fprintf(stdout, "%s: graph_init failed!\n", __FUNCTION__);
		return -1;
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
//main
int test_graph_main(int argc, char ** argv)
{
	test_graph_1_main(argc, argv);
	test_graph_init_from_file_main(argc, argv);
	return 0;
}
