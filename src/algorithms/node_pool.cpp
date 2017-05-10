#include "node_pool.h"
#include "rb_tree.h"	/* rbtree_node */
#include <stdio.h>

#define RTR_POOL_N (32 * 1024 / sizeof(rbtree_node))

rbtree_node * node_new(node_pool & p, void * data)
{
//	fprintf(stdout, "%s:\n", __FUNCTION__);
	if(p.i == POOL_N){
		fprintf(stderr, "%s: out of memory\n", __FUNCTION__);
		exit(0);
	}

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (rbtree_node * )calloc(RTR_POOL_N, sizeof(rbtree_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto ret = &((rbtree_node **)p.n)[p.i - 1][p.j++];
	ret->left = ret->right = ret->p = 0;
	ret->data = data;
	ret->red = false;

	return ret;
}

void node_pool_init(node_pool & p)
{
	p.i = p.j = 0;
}
