#include "graph.h"       /* graph */
#include "string_util.h" /* strnrchr */
#include <stdlib.h>
#include <string.h>		 /* memmove */

/////////////////////////////////////////////////////////////////////////////////////
graph_node ** graph_vertex(graph const& g, graph_node ** v, size_t & n)
{
	rbtree_inorder_walk(g.tr, (void **)v, n);
	for(size_t i = 0; i < n; ++i)
		v[i] = ((graph_bag *)v[i])->node;
	return v;
}

graph_bag * graph_search(graph const& g, int key)
{
	graph_node gn{ key };
	graph_bag bnode { &gn };
	auto node = (rbtree_node *)rbtree_search(g.tr, &bnode);

	if(!(node && node->data))
		return 0;

	return ((graph_bag * )node->data);
}

int graph_add_edge(graph & g, int v, int w, double weight/* = 0*/)
{
	auto nodev = graph_search(g, v);
	auto nodew = graph_search(g, w);
	if(!(nodev && nodew && nodev->node && nodew->node))
		return -1;

	g.graph_bag_realloc(nodev, g.weight);
	if(!g.direct)
		g.graph_bag_realloc(nodew, g.weight);

	nodev->v[nodev->i] = nodew->node;
	if(g.weight){
		nodev->w[nodev->i] = weight;
	}
	++nodev->i;

	if(!g.direct){
		nodew->v[nodew->i] = nodev->node;
		if(g.weight)
			nodew->w[nodew->i] = weight;
		++nodew->i;
	}
	return 0;
}

int graph_add_vertex(graph & g, int v)
{
	auto gn = g.graph_node_new(g.npool, v);
	auto gb = g.graph_bag_new(g.bpool, gn, g.v, g.weight);
	if(!(gn && gb))
		return -1;

	return rbtree_insert(g.tr, gb)? 0 : -1;
}

graph_node ** graph_adj(graph const& g, int v, size_t & sz)
{
	auto gb = graph_search(g, v);
	if(!(gb && gb->node)){
		sz = 0;
		return 0;
	}
	sz = gb->i;
	return gb->v;
}

static void do_graph_simple_dfs(graph & g, int v, FILE * out)
{
	g.dfs_marked[v] = true;
	fprintf(out, "%d,", v);

	size_t sz;
	auto adj = graph_adj(g, v, sz);
	for(size_t i = 0; i < sz; ++i){
		if(!g.dfs_marked[adj[i]->key])
			do_graph_simple_dfs(g, adj[i]->key, out);
	}
}

void graph_simple_dfs(graph & g, int v, FILE * out)
{
	g.dfs_marked = (bool *)realloc(g.dfs_marked, g.v * sizeof(bool));
	memset(g.dfs_marked, 0, g.v * sizeof(bool));

	return do_graph_simple_dfs(g, v, out);
}

char * graph_c_str(graph const& g, char * buf, size_t& len)
{
	if(!buf)
		len = 0;

//	printf("%s: begin rbtree_inorder_walk\n", __FUNCTION__);
	void * vers[g.v];
	size_t vlen;
	rbtree_inorder_walk(g.tr, vers, vlen);
//	printf("%s: end rbtree_inorder_walk\n", __FUNCTION__);

	char tmp[128];

	char * buffer;
	size_t length;

	size_t n = 0;
//	char buff[64];
	for(size_t i = 0; i < vlen; ++i){

//		if((i + 1) % 10000 == 0)
//			printf("\r%s: processed %zu vertex, strlen=%zu/%s", __FUNCTION__,
//					i + 1, n, byte_to_mb_kb_str_r(n, "%-.2f %cB", buff));

		auto gb = (graph_bag *)vers[i];
		if(!gb || !gb->node || gb->i == 0)
			continue;

		if(buf && len > n){
			buffer = buf + n;
			length = len - n;
		}
		else{
			buffer = tmp;
			length = sizeof(tmp);
		}

		auto r = snprintf(buffer, length, "\t%d: ", gb->node->key);
		if(r < 0)
			return 0;

		n += r;

		for(size_t j = 0; j < gb->i; ++j){
			if(buf && len > n){
				buffer = buf + n;
				length = len - n;
			}
			else{
				buffer = tmp;
				length = sizeof(tmp);
			}

			int r2;
			if(g.weight)
				r2 = snprintf(buffer, length, "(%f)%d, ", gb->w[j], gb->v[j]->key);
			else
				r2 = snprintf(buffer, length, "%d, ", gb->v[j]->key);

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
//	fprintf(stdout, "%s: vertex='%zu', strlen=%zu\n", __FUNCTION__, vlen, n);

	return buf;
}

int graph_init(graph & g, FILE * in)
{
	/* vertex and edge count */
	auto r = fscanf(in, "%zu", &g.v);
	if(r != 1) return -1;
	r = fscanf(in, "%zu", &g.e);
	if(r != 1) return -1;

//	fprintf(stderr, "%s: vertex=%zu, edge=%zu\n", __FUNCTION__, g.v, g.e);

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

/* edge_weighted_graph init, @see graph_init */
int wgraph_init(graph & g, FILE * in,
		void(* cb_vertex_begin)(size_t total, size_t n)/* = 0*/,
		void(* cb_vertex_end)(size_t total, size_t n)/* = 0*/,
		void(* cb_edge_begin)(size_t total, size_t n)/* = 0*/,
		void(* cb_edge_end)(size_t total, size_t n)/* = 0*/)
{
	fprintf(stdout, "%s: \n", __FUNCTION__);
	g.weight = true;
	/* vertex and edge count */
	auto r = fscanf(in, "%zu", &g.v);
	if(r != 1) return -1;
	r = fscanf(in, "%zu", &g.e);
	if(r != 1) return -1;

	fprintf(stderr, "%s: vertex=%zu, edge=%zu\n", __FUNCTION__, g.v, g.e);

	size_t i = 0;
	for(; i < g.v; ++i){
		auto r = graph_add_vertex(g, i);
		if(r != 0)
			return -1;
		if(cb_vertex_begin)
			cb_vertex_begin(g.v, i + 1);

	}
	cb_vertex_end(g.v, i);
	/* load edge data */
	size_t n = 0;
	int w, x;
	float we;
	for(int r; (r = fscanf(in, "%d%d%f",  &w, &x, &we)) != EOF; ){
		if(r != 3)
			continue;

		if(graph_add_edge(g, w, x, we) != 0){
			fprintf(stderr, "%s: graph_add_edge for %d-%d failed, skip\n", __FUNCTION__, w, x);
		}

		++n;

		if(cb_edge_begin)
			cb_edge_begin(g.e, n + 1);
	}
	cb_edge_end(g.e, n);
	return 0;

}

graph dgraph_reverse_copy(graph const& og)
{
	graph g{0};
	if(!og.direct)
		return g;

	g = og;
	node_pool_init(g.tr.pool);
	g.tr.root = 0;
	node_pool_init(g.npool);
	node_pool_init(g.bpool);
	g.dfs_marked = 0;

	for(size_t i = 0; i < og.v; ++i){
		auto r = graph_add_vertex(g, i);
		if(r != 0)
			return graph{0};
	}

	graph_node * vers[g.v];
	size_t V;
	if(!graph_vertex(og, vers, V) || V == 0)
		return graph{0};

	for(size_t i = 0; i < V; ++i){
		if(!vers[i])
			continue;

		size_t sz;
		auto adj = graph_adj(og, vers[i]->key, sz);
		if(!adj || sz == 0)
			continue;

		for(size_t j = 0; j < sz; ++j){
			if(!adj[j])
				continue;
			graph_add_edge(g, adj[j]->key, vers[i]->key);
		}
	}
	return g;
}

graph & dgraph_reverse(graph & g)
{
	if(!g.direct)
		return g;
	return g;
}
