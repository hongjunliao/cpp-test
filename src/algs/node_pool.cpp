/*!
 * @author hongjun.liao <docici@126.com>
 */
#include "node_pool.h"
#include "rb_tree.h"	/* rbtree_node */
#include <stdio.h>
#include <math.h>       /* log2 */

#define RTR_POOL_N (1024 * 1024 * 8 / sizeof(rbtree_node))

rbtree_node * node_new(node_pool & p, void * data)
{
	if(!p.n[0]){
		p.n[0] = (rbtree_node * )malloc(RTR_POOL_N * sizeof(rbtree_node));
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

	if(p.j == RTR_POOL_N - 1){
//		fprintf(stdout, "%s: N=%zu, i=%zu, j=%zu\n", __FUNCTION__, p.N, p.i, p.j);
		++p.i;
		p.n[p.i] = (rbtree_node * )malloc(RTR_POOL_N * sizeof(rbtree_node));
		if(!p.n[p.i]){
			fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
			exit(0);
		}
		p.j = 0;
	}

	auto node = &((rbtree_node **)p.n)[p.i][p.j++];
	node->left = node->right = node->p = 0;
	node->data = data;
	node->red = false;

	return node;
}

void node_pool_init(node_pool & p, size_t n/* = 8*/)
{
	p.i = p.j = 0;
	p.N = n;
	p.n = (void **)calloc(p.N, sizeof(void *));
}
