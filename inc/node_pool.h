/*!
 * test for memory poll, currently for tree nodes
 * @author hongjun.liao <docici@126.com>, @date 2017//05/03
 *
 */
#ifndef HONGJUN_LIAO_NODE_POOL_H_
#define HONGJUN_LIAO_NODE_POOL_H_

#include <stdlib.h>

#define POOL_N 512
struct node_pool {
	void * n[POOL_N];
	size_t i;	/* row */
	size_t j;   /* col */
};

struct rbtree_node;

void node_pool_init(node_pool & p);
rbtree_node * node_new(node_pool & p, void * data);

#endif /* HONGJUN_LIAO_NODE_POOL_H_ */
