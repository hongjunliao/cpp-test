#include "graph.h"       /* graph */
#include "string_util.h" /* strnrchr */
#include <stdlib.h>
#include <string.h>		 /* memmove */

/////////////////////////////////////////////////////////////////////////////////////
graph_bag * graph_search(graph const& g, int key)
{
	graph_node gn{ key };
	graph_bag bnode { &gn };
	auto node = (rbtree_node *)rbtree_search(g.tr, &bnode);

	if(!(node && node->data))
		return 0;

	return ((graph_bag * )node->data);
}

int graph_add_edge(graph & g, int v, int w)
{
	auto nodev = graph_search(g, v);
	auto nodew = graph_search(g, w);
	if(!(nodev && nodew && nodev->node && nodew->node))
		return -1;

	nodev->v[nodev->i++] = nodew->node;
	if(!g.is_direct)
		nodew->v[nodew->i++] = nodev->node;
	return 0;
}

int graph_add_vertex(graph & g, int v)
{
	auto gn = g.graph_node_new(g.npool, v);
	auto gb = g.graph_bag_new(g.bpool, gn, g.v);
	if(!(gn && gb))
		return -1;

	return rbtree_insert(g.tr, gb)? 0 : -1;
}

graph_node ** graph_adj(graph const& g, int v, int & sz)
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

	int sz;
	auto adj = graph_adj(g, v, sz);
	for(int i = 0; i < sz; ++i){
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

	void * vers[g.v];
	int vlen;
	rbtree_inorder_walk(g.tr, vers, vlen);

	char tmp[128];
	size_t n = 0;
	for(int i = 0; i < vlen; ++i){
		auto gb = (graph_bag *)vers[i];
		if(!gb || !gb->node || gb->i == 0)
			continue;

		auto r = buf && len > n?
				snprintf(buf + n, len - n, "\t%d: ", gb->node->key):
				snprintf(tmp, 128, "\t%d: ", gb->node->key);

		if(r < 0)
			return 0;

		n += r;

		for(auto j = 0; j < gb->i; ++j){
			auto r2 = buf && len > n?
					snprintf(buf + n, len - n, "%d, ", gb->v[j]->key):
					snprintf(tmp, 128, "%d, ", gb->v[j]->key);

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
