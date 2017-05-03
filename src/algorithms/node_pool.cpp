#include "node_pool.h"
#include "rb_tree.h"	/* rbtree_node */
#include <stdio.h>

rbtree_node * node_alloc(node_pool & p)
{
//	fprintf(stdout, "%s:\n", __FUNCTION__);
	if(p.i == 32){
		fprintf(stderr, "%s: outof memory\n", __FUNCTION__);
		return 0;
	}
	auto RTR_POOL_N = (32 * 1024 / sizeof(rbtree_node));

	if(p.i == 0 || p.j == RTR_POOL_N)
		p.n[p.i++] = (rbtree_node * )calloc(RTR_POOL_N, sizeof(rbtree_node));

	if(p.j == RTR_POOL_N)
		p.j = 0;

	auto pp = (rbtree_node **)p.n;
	auto ret = &pp[p.i - 1][p.j++];

//	fprintf(stdout, "%s:\n", __FUNCTION__);
	return ret;
}

