/*!
 * test for memory poll, currently for tree nodes
 * @author hongjun.liao <docici@126.com>, @date 2017//05/03
 *
 */
#ifndef HONGJUN_LIAO_NODE_POOL_H_
#define HONGJUN_LIAO_NODE_POOL_H_

#include <stdlib.h>

struct node_pool {
	void * n[32];
	size_t i;	/* row */
	size_t j;   /* col */
};

struct rbtree_node;

rbtree_node * node_alloc(node_pool & p, int key, bool red);

#endif /* HONGJUN_LIAO_NODE_POOL_H_ */
