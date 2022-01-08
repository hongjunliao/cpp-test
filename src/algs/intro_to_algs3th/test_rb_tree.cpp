/*!
 * test rb_tree/red_black_tree, from book <Introduction to Algorithms, 3th Edition>_13
 * @author hongjun.liao <docici@126.com>, @date 2017//04/30
 *
 * 红黑性质:
　*　(1)节点是红色或黑色。
 * (2)根节点是黑色。
 * (3)每个叶节点（NIL节点，空节点）是黑色的。
 * (4)每个红色节点的两个子节点都是黑色。(从每个叶子到根的所有路径上不能有两个连续的红色节点)
 * (5)从任一节点到其每个叶子的所有路径都包含相同数目的黑色节点。
 *
 * NOTES:
 * (1)红黑二叉查找树背后的基本思想是用标准的二叉查找树(完全由2-结点构成)和一些额外的信息(替换3-结点)来表示2-3树.
 * (2)我们将树中的链接分为两种类型:红链接将两个2-结点连接起来构成一个3-结点, 黑链接则是2-3树中的普通链接;
 *    确切地说, 我们将3-结点表示为由一条左斜的红色链接相连的两个2-结点;
 *    我们将用这种方式表示2-3树的二叉查找树称为红黑二叉查找树(简称红黑树)
 * (3)一个结点的颜色指的是指向该结点的链接的颜色
 */

#include <stdio.h>
#include <string.h>	    /* strlen */
#include <string>	  /* std::string */
#include <queue>	  /* std::queue */
#include <vector>	  /* std::vector */
#include <algorithm>  /* std::max_element */

#include "node_pool.h"	/* node_pool */

struct rbtree_node {
	rbtree_node * left;     /* left child */
	rbtree_node * right;    /* right child */
	rbtree_node * p;	    /* parent */

	int key;
	bool red;				/* true if red, else black */
};

struct node_pool;

struct rb_tree {
	rbtree_node * root;
	node_pool pool;
	rbtree_node * (*node_new)(node_pool & p, int key, bool red);
	void (*node_del)(rbtree_node *);
};

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
static rbtree_node * rbtree_insert(rb_tree & tr, int key)
{
	/* empty tree, first add root and return */
	if(!tr.root){
		auto node = tr.node_new(tr.pool, key, false);	/* root always black */
		tr.root = node;
		return node;
	}

	/* tree NOT empty, find insert position */
	rbtree_node * y = 0;
	for(auto x = tr.root; x; ){
		y = x;
		if(key < x->key)
			x = x->left;
		else
			x = x->right;
	}

	/* insert the new node */
	auto node = tr.node_new(tr.pool, key, true); /* new rode is always red */
	if(key < y->key)
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

static void rbtree_inorder_walk(rbtree_node const * root)
{
	if(!root)
		return;

	rbtree_inorder_walk(root->left);
	fprintf(stdout, "%c, ", root->key);
	rbtree_inorder_walk(root->right);
}

static void rbtree_init(char const  * buf, size_t len, rb_tree & tr)
{
	for(size_t i = 0; i < len; ++i){
		rbtree_insert(tr, buf[i]);
	}
}

static void rbtree_init(FILE * in, rb_tree & tr)
{
	char buf[1024];
	while(fgets(buf, sizeof(buf), in)){
		rbtree_init(buf, strlen(buf) - 1, tr);
	}
}

static rbtree_node * test_rbnode_new(node_pool & p, int key, bool red)
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

	auto ret = &((rbtree_node **)p.n)[p.i - 1][p.j++];
	ret->left = ret->right = ret->p = 0;
	ret->key = key;
	ret->red = red;

	return ret;
}
/* print bstree to terminal, like this:
 * H
 * C(H), (H)R
 * A(C), (C)E, M(R), (R)X
 * L(M), (M)P, S(X)
 */
static void rbtree_draw_to_term(rb_tree const& tr)
{
	fprintf(stdout, "%s: \n", __FUNCTION__);

    int curLayerCount = 0; 	//当前层中的节点数
    int nextLayerCount = 0; //下一层中的节点数

    std::queue<rbtree_node *> nodes;
    nodes.push(tr.root);

    curLayerCount++;
    while(!nodes.empty()){
    	auto p = nodes.front();
    	nodes.pop();
        curLayerCount--;

        if(!p->p)
        	fprintf(stdout, "%c", p->key);
        else if(p->p->left == p)
        	fprintf(stdout, "%c(%c) ", p->key, p->p->key);
        else if(p->p->right == p)
        	fprintf(stdout, "(%c)%c ", p->p->key, p->key);
        else
        	fprintf(stdout, "%c", p->key);

        if (p->left){
            nodes.push(p->left);
            nextLayerCount++;
        }
        if (p->right){
        	nodes.push(p->right);
            nextLayerCount++;
        }
        if (curLayerCount == 0) { //一层已经遍历完毕
            curLayerCount = nextLayerCount;
            nextLayerCount = 0;

            fprintf(stdout, "\n");
        }
    }
}

/* shell cmd: echo "SEARCHXMPL" | ./cpp-test-main rbtree */
int test_rbtree_main(int argc, char ** argv)
{
//	int keys[] = {'S', 'E', 'A', 'R', 'C', 'H', 'X', 'M', 'P', 'L', };
//	int keys[] = {'A', 'C', 'E', 'H', 'L', 'M', 'P', 'R', 'S', 'X', };
//	auto keys = "SEARCHXMPL";
//	auto keys = "ACEHLMPRSX";
	rb_tree tr { 0 };
	node_pool_init(tr.pool);
	tr.node_new = test_rbnode_new;

	rbtree_init(stdin, tr);
//	for(auto i : keys)
//		rbtree_insert(tr, i);


	if(tr.root){
		rbtree_draw_to_term(tr);

		fprintf(stdout, "%s: rbtree_inorder_walk, root=%c, [", __FUNCTION__, tr.root->key);
		rbtree_inorder_walk(tr.root);
		fprintf(stdout, "]\n");
	}
	else
		fprintf(stdout, "empty rbtree\n");

	return -1;
}
