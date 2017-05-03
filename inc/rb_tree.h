/*!
 * test rb_tree/red_black_tree, from book <Introduction to Algorithms, 3th Edition>_13
 * @author hongjun.liao <docici@126.com>, @date 2017//05/03
 */

#ifndef HONGJUN_LIAO_RB_TREE_H_
#define HONGJUN_LIAO_RB_TREE_H_

struct rbtree_node {
	rbtree_node * left;     /* left child */
	rbtree_node * right;    /* right child */
	rbtree_node * p;	    /* parent */

	bool red;				/* true if red, else black */

	int key;
};

#define RBTREE_NODE(key)   (rbtree_node{ 0, 0, 0, false, key })

struct node_pool;

struct rb_tree {
	rbtree_node * root;
	node_pool pool;
	rbtree_node * (*node_alloc)(node_pool & p);
};

#endif /* HONGJUN_LIAO_RB_TREE_H_ */
