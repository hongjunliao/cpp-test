/*!
 * test rb_tree/red_black_tree, from book <Introduction to Algorithms, 3th Edition>_13
 * @author hongjun.liao <docici@126.com>, @date 2017//05/03
 */

#ifndef HONGJUN_LIAO_RB_TREE_H_
#define HONGJUN_LIAO_RB_TREE_H_

#include "../node_pool.h"	/* node_pool */

struct rbtree_node {
	rbtree_node * left;     /* left child */
	rbtree_node * right;    /* right child */
	rbtree_node * p;	    /* parent */
	bool red;				/* true if red, else black */

	void * data;			/* data of node */
};

struct node_pool;

struct rb_tree {
	rbtree_node * root;
	node_pool     pool;

	rbtree_node * (* node_new)(node_pool & p, void * data);
	void (* node_del)(rbtree_node * node);
	/* @return: if @param a < @param b, then <0, == then 0, else >0 */
	int (* node_cmp)(void const * a, void const * b);
	/* to string */
	char const * (* node_c_str)(void const * a, char * buf, size_t len);

	/* free node data */
	void (* node_data_free)(void * data);
};

rbtree_node * rbtree_insert(rb_tree & tr, void * data);
rbtree_node * rbtree_search(rb_tree const& tr, void const* data);

void rbtree_inorder_walk(rb_tree const& tr);
void rbtree_inorder_walk(rb_tree const& tr, void ** nodes, size_t& len);

#endif /* HONGJUN_LIAO_RB_TREE_H_ */
