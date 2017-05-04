/*!
 * test rb_tree/red_black_tree, from book <Introduction to Algorithms, 3th Edition>_13
 * @author hongjun.liao <docici@126.com>, @date 2017//05/03
 */

#ifndef HONGJUN_LIAO_RB_TREE_H_
#define HONGJUN_LIAO_RB_TREE_H_

#include "node_pool.h"	/* node_pool */

struct rbtree_node {
	rbtree_node * left;     /* left child */
	rbtree_node * right;    /* right child */
	rbtree_node * p;	    /* parent */

	int key;
	bool red;				/* true if red, else black */
};

#define RBTREE_NODE(key)   (rbtree_node{ 0, 0, 0, key, true })

struct node_pool;

struct rb_tree {
	rbtree_node * root;
	node_pool pool;
	rbtree_node * (*node_alloc)(node_pool & p, int key, bool red);
};

#endif /* HONGJUN_LIAO_RB_TREE_H_ */
