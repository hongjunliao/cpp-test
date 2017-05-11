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
	double weight;	   /* weight */
};

/* TODO: Y this call bag in <Algorithms,4th Edition>? */
struct graph_bag {
	graph_node * node; /* this node */
	graph_node ** v;   /* adjs for this node*/
	int i;             /* index for adjs */
};

struct graph {
	rb_tree tr;        /* nodes */
	node_pool npool;    /* for graph_node */
	node_pool bpool;   /* for graph_bag */
	size_t v;		   /* vertex */
	size_t e;          /* edges */

	bool is_direct;    /* directed_graph? */
	bool * dfs_marked; /* for dfs */

	graph_node * (*graph_node_new)(node_pool & pool, int key);
	graph_bag * (*graph_bag_new)(node_pool & pool, graph_node * node, size_t v);
};

/////////////////////////////////////////////////////////////////////////////////////
/* @param key:  key for graph_node */
graph_bag * graph_search(graph const& g, int key);
/* @return: 0 on success */
int graph_add_edge(graph & g, int v, int w);
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
char * graph_c_str(graph const& g, char * buf, size_t& len);
/* init graph @param g from file @param f, file @param f has the following format:
 * vertex_total         <---total vertex
 * edge_total           <---total edges
 * vertex1 vertex2      <---one edge
 * vertex1 vertex2      <---another edge
 * ...                  <---...(other edges)
 *
 * sample('\n' means newline):
 * 13\n13\n0 5\n4 3\n0 1\n9 12\n6 4\n5 4\n0 2\n11 12\n9 10\n0 6\n7 8\n9 11\n5 3
 *
 * @return: 0 on success
 * */
int graph_init(graph & g, FILE * in);
#endif /* DOCICI_GRAPH_H_ */
