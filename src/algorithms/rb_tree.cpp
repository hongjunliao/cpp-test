/*!
 * red_black_tree, currently only support insert and search
 * @author hongjun.liao <docici@126.com>, @date 2017//05/06
 */

#include "rb_tree.h"	/* rb_tree */
#include <stdio.h>
#include <string.h>	    /* strlen */
/* left_rotate
 * NOTES: @param node MUST have right_child
 * @return root node of the rotated subtree
 * */
static rbtree_node * rbtree_left_rotate(rb_tree & tr, rbtree_node * x);
static rbtree_node * rbtree_right_rotate(rb_tree & tr, rbtree_node * x);

/* sample bstree left_rotate, for node 11:
 *            ___7___                           ___7_____
 *           /       \                         /         \
 *          4       __11___                   4         _18__
 *         / \     /       \                 / \       /     \
 *        3   6   9      __18__      ==>    3   6     11      19
 *       /              /      \           /         /  \       \
 *      2              14      19         2         9   14       22
 *                    /  \       \                     /   \    /
 *                   12  17      22                   12   17  20
 *                              /
 *                             20
 */
static rbtree_node * rbtree_left_rotate(rb_tree & tr, rbtree_node * x)
{
	if(!x || !x->right)
		return x;

	auto y = x->right;
	/* turn y's left to x's right */
	x->right = y->left;
	if(y->left)
		y->left->p = x;
	/* linkx x'parent to y */
	y->p = x->p;
	if(!x->p)
		tr.root = y;
	else if(x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;
	/* put x on y's left */
	y->left = x;
	x->p = y;

	y->red = x->red;
	x->red	= true;

	return y;
}

static rbtree_node * rbtree_right_rotate(rb_tree & tr, rbtree_node * x)
{
	if(!x || !x->left)
		return x;
	auto y = x->left;
	x->left = y->right;
	if(y->right)
		y->right->p = x;
	y->p = x->p;
	if(!x->p)
		tr.root = y;
	else if(x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;

	y->right = x;
	x->p = y;

	y->red = x->red;
	x->red = true;

	return y;
}

static void rbtree_flipcolor(rbtree_node * node)
{
	node->red = (node->p);
	node->left->red = node->right->red = false;
}

static inline bool rbtree_is_red(rbtree_node * node)
{
	return node && node->red;
}

/* rbtree insert operation
 * insert and return the new node */
rbtree_node * rbtree_insert(rb_tree & tr, void * data)
{
	/* empty tree, first add root and return */
	if(!tr.root){
		auto node = tr.node_new(tr.pool, data);
		node->red = false;	/* root always black */

		tr.root = node;
		return node;
	}

	/* tree NOT empty, find insert position */
	rbtree_node * y = 0;
	for(auto x = tr.root; x; ){
		y = x;
		auto r = tr.node_cmp(data, x->data);
		if(r == 0){
			tr.node_del(x);
			x->data = data;
			return x;
		}
		if(r < 0)
			x = x->left;
		else
			x = x->right;
	}

	/* insert the new node */
	auto node = tr.node_new(tr.pool, data);
	node->red = true;

	if(tr.node_cmp(data, y->data) < 0)
		y->left = node;
	else
		y->right = node;
	node->p = y;

	/* rotate and change colors, to make it a rbtree */
	for(auto p = node->p; p;){
		/* (1)new node is on right of an 3-node */
		if(rbtree_is_red(p->left) && rbtree_is_red(p->right)){
			rbtree_flipcolor(p);
			p = p->p; /* (2)transmit red to parent */
			continue;
		}
		/* (3)new node is on left of an 3-node, change to (1) */
		if(rbtree_is_red(p->left) && (p->red && p->p && p->p->left == p)){
			p = rbtree_right_rotate(tr, p->p);
			continue;
		}
		/* (4)new node is on middle of an 3-node, change to (3) */
		if(rbtree_is_red(p->right) && (p->red && p->p && p->p->left == p) ){
			p = rbtree_left_rotate(tr, p);
			continue;
		}
		/* new node is on right of an 2-node */
		if(rbtree_is_red(p->right)){
			p = rbtree_left_rotate(tr, p);
			continue;
		}
		/* reached a none-3-node, just break */
		break;
	}

	return node;
}
