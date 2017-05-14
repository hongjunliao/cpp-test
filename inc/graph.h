/*!
 * graph, from book <Algorithms 4th Edition>_4
 * @author hongjun.liao <docici@126.com>, @date 2017//05/11
 *
 * visualization, see http://www.cs.usfca.edu/~galles/visualization/DFS.html
 */
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef DOCICI_GRAPH_H_
#define DOCICI_GRAPH_H_

#include "rb_tree.h"	 /* rb_tree */
#include "node_pool.h"	 /* node_pool */
#include <stdio.h>

struct graph_node;
struct graph_bag;
struct graph;

struct graph_node {
	int key;           /* node key */
};

/* TODO: Y this call bag in <Algorithms,4th Edition>?
 * edge: node->v[i](begin_node->end_node)
 * */
struct graph_bag {
	graph_node * node; /* this node */
	graph_node ** v;   /* adjs for this node, v[i] */
	double * w;        /* weight for edges, w[i] */
	int i;             /* index for v and w */
	int I;             /* max of v and w */
};

struct graph {
	rb_tree tr;        /* nodes */
	node_pool npool;   /* for graph_node */
	node_pool bpool;   /* for graph_bag */
	size_t v;		   /* vertex */
	size_t e;          /* edges */

	bool direct;       /* directed_graph? */
	bool weight;       /* weight_graph? */
	bool * dfs_marked; /* for dfs */

	graph_node * (*graph_node_new)(node_pool & pool, int key);
	graph_bag * (*graph_bag_new)(node_pool & pool, graph_node * node, size_t v, bool weight);
	void (*graph_bag_realloc)(graph_bag * gb, bool weight);
};

/////////////////////////////////////////////////////////////////////////////////////
/* @param key:  key for graph_node */
graph_bag * graph_search(graph const& g, int key);

/*
 * @param weight: weight for v->w
 * @return: 0 on success */
int graph_add_edge(graph & g, int v, int w, double weight = 0);

int graph_add_vertex(graph & g, int v);
/* search adj for vertex @param v */
graph_node ** graph_adj(graph const& g, int v, int & sz);

/* simple depth first search for graph */
void graph_simple_dfs(graph & g, int v, FILE * out);
/* graph to string, output format:
 * vertex0: vertex0, vertex1, vertex, ...
 * vertex1: vertex0, vertex1, ...
 * ...
 *
 * sample 1(without weight):
 *  0: 5, 1, 2, 6,
 *	4: 3,
 *	5: 4, 3,
 *	6: 4,
 *	7: 8,
 *	9: 12, 10, 11,
 *	11: 12,
 *  ...
 *
 * sample 2(with weight)
 * 0: (0.160000)7, (0.380000)4, (0.260000)2, (0.580000)6,
 * 1: (0.320000)5, (0.190000)7, (0.360000)2, (0.290000)3,
 * 2: (0.170000)3, (0.260000)0, (0.360000)1, (0.340000)7, (0.400000)6,
 * ...
 *
 * @return:
 * if actual length of graph > @param len or error ocurred, then return NULL
 * @param len set to miminal length of graph or set to 0 if error
 * */
char * graph_c_str(graph const& g, char * buf, size_t& len);
/* init graph @param g from file @param in, file @param in has the following format:
 * vertex_total         <---total vertex
 * edge_total           <---total edges
 * vertex vertex        <---one edge
 * vertex vertex        <---another edge
 * ...                  <---...(other edges)
 *
 * sample('\n' means newline):
 * 13\n13\n0 5\n4 3\n0 1\n9 12\n6 4\n5 4\n0 2\n11 12\n9 10\n0 6\n7 8\n9 11\n5 3
 *
 * @return: 0 on success
 * */
int graph_init(graph & g, FILE * in);
/* edge_weighted_graph init, same as graph_init, except that with weight,
 * file @param in has the following format:
 * vertex_total            <---total vertex
 * edge_total              <---total edges
 * vertex vertex weight    <---one edge
 * vertex vertex weight    <---another edge
 * ...                     <---...(other edges)
 *
 * sample:
 * 8
 * 16
 * 4 5 0.35
 * 4 7 0.37
 * ...
 *
 * @param cb_vertex_begin, cb_vertex_end, cb_edge_begin, cb_edge_end:
 * callbacks while processing
 * @return: 0 on success
 *  */
int wgraph_init(graph & g, FILE * in,
		void(* cb_vertex_begin)(size_t total, size_t n) = 0,
		void(* cb_vertex_end)(size_t total, size_t n) = 0,
		void(* cb_edge_begin)(size_t total, size_t n) = 0,
		void(* cb_edge_end)(size_t total, size_t n) = 0);
#endif /* DOCICI_GRAPH_H_ */
