//============================================================================
// Name        : test_binary_search_tree.cpp
// Author      : hongjun.liao <docici@126.com>
// Version     :
// Copyright   :
// Description : test_binary_search_tree/bst
//============================================================================
#include <stdio.h>
#include <stdlib.h>	/* rand */
#include <time.h>	/* time */
#include <initializer_list> /* std::initializer_list */
#include <map>


struct bstree_node {
	int key;
	bstree_node * left;  /* left child */
	bstree_node * right; /* right child */

	bstree_node * p;	    /* parent */
};

/* NOTE: max always in right-most, with no right child! */
bstree_node const * bstree_max(bstree_node const * root)
{
	while(root->right)
		root = root->right;
	return root;
}

/* NOTE: min always in left-most, with no left child! */
bstree_node * bstree_min(bstree_node * root)
{
	while(root->left)
		root = root->left;
	return root;
}

/* @param root maybe null, in this condition, @param node becomes the root
 * @return: 0 on success */
int bstree_insert(bstree_node *& root, bstree_node * node)
{
	if(!node)
		return -1;

	bstree_node * p = 0;
	for(auto x = root; x; ){
		p = x;

		if(node->key < p->key)
			x = x->left;
		else
			x = x->right;
	}
	if(!p)
		root = node;
	else if(node->key < p->key)
		p->left = node;
	else
		p->right = node;

	node->p = p;

	return 0;
}

bstree_node const * bstree_search(bstree_node const * root, int key)
{
	auto x = root;
	for(; x && x->key != key; ){
		if(key < x->key)
			x = x->left;
		else
			x = x->right;
	}
	return x;
}

/* override version, recursive */
bstree_node const * bstree_search_recursive(bstree_node const * root, int key)
{
	if(!root || key == root->key)
		return root;

	if(key <= root->key)
		return bstree_search(root->left, key);

	return bstree_search(root->right, key);
}

static void bstree_transplant(bstree_node *& root, bstree_node const * u, bstree_node * v)
{
	if(!u->p)
		root = v;
	else{
		auto & n = u->p->left == u? u->p->left : u->p->right;
		n = v;
	}

	if(v)
		v->p = u->p;
}

/* @param node,   must NOT null */
void bstree_delete(bstree_node *& root, bstree_node const * node)
{
	if(!node->left){	/* no left_child, replace with right_child(maybe null) */
		bstree_transplant(root, node, node->right);
		return;
	}
	if(!node->right){ /* only left_child, replace with left_child */
		bstree_transplant(root, node, node->left);
		return;
	}
	/* both left and right child */
	auto y = bstree_min(node->right);	/* NOTE: y has no left child, @see bstree_min */

	if(y->p != node){ /* y is NOT the right child of node */
		bstree_transplant(root, y, y->right);
		y->right = node->right;
		y->right->p = y;
	}

	bstree_transplant(root, node, y);
	y->left = node->left;
	y->left->p = y;
}

/* a sample bstree,
 * if we put all g_nodes to one line, from left node to right node:
 *            ___5___
 *           /       \
 *          3         9
 *         /  \      /  \
 *        1    4    7   12
 *        | |  | |  | | |
 *        | |  | |  | | |
 *        1 3  4 5  7 9 12
 * as you can see, all g_nodes sorted ASC, this is "inorder walk" !!!
 */
void bstree_inorder_walk(bstree_node const * root)
{
	if(!root)
		return;

	bstree_inorder_walk(root->left);
	fprintf(stdout, "%d, ", root->key);
	bstree_inorder_walk(root->right);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
static int bstree_init(bstree_node *& tr, bstree_node * nodes, size_t n);

/* verify if  @param root is a bst */
static int bstree_debug_verrify(bstree_node const * root);

static int test_binary_search_tree_search(int argc, char ** argv)
{
	bstree_node nodes[] = { {100}, {90}, {95}, {80}, {70}, {85}, {94}, {110}, {105}, {120},
			{103}, {108}, {115}, {118}, {119}, {130}, {140} };
	static size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bstree_node * tr;
	auto r = bstree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	auto maxn = bstree_max(tr);
	fprintf(stdout, "%s: bstree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = bstree_min(tr);
	fprintf(stdout, "%s: bstree_min=%d\n", __FUNCTION__, minn->key);

	return 0;
}

/* delete node with no left child */
static int test_binary_search_tree_delete_1(int argc, char ** argv)
{
	bstree_node nodes[] = { {100}, {90}, {95}, {80}, {70}, {85}, {94}, {110}, {105}, {120},
			{103}, {108}, {115}, {118}, {119}, {130}, {140} };
	static size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bstree_node * tr;
	auto r = bstree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	int key = 118;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);

	if(n){
		fprintf(stdout, "%s: bstree_delete key=%d\n", __FUNCTION__, n->key);
		bstree_delete(tr, n);
		fprintf(stdout, "%s: bstree_inorder_walk: root=%d, [", __FUNCTION__, tr->key);
		bstree_inorder_walk(tr);
		fprintf(stdout, "]\n");
	}
	return 0;
}

/* delete node with only left child */
static int test_binary_search_tree_delete_2(int argc, char ** argv)
{
	bstree_node nodes[] = { {100}, {90}, {95}, {80}, {70}, {85}, {94}, {110}, {105}, {120},
			{103}, {108}, {115}, {118}, {119}, {130}, {140} };
	static size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bstree_node * tr;
	auto r = bstree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	int key = 95;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);

	if(n){
		fprintf(stdout, "%s: bstree_delete key=%d\n", __FUNCTION__, n->key);
		bstree_delete(tr, n);
		fprintf(stdout, "%s: bstree_inorder_walk: root=%d, [", __FUNCTION__, tr->key);
		bstree_inorder_walk(tr);
		fprintf(stdout, "]\n");
	}

	return 0;
}

/* delete node with y is NOT the right child of node*/
static int test_binary_search_tree_delete_3(int argc, char ** argv)
{
	bstree_node nodes[] = { {100}, {90}, {95}, {80}, {70}, {85}, {94}, {110}, {105}, {120},
			{103}, {108}, {115}, {118}, {119}, {130}, {140} };
	static size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bstree_node * tr;
	auto r = bstree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	int key = 110;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);

	if(n){
		fprintf(stdout, "%s: bstree_delete key=%d\n", __FUNCTION__, n->key);
		bstree_delete(tr, n);
		fprintf(stdout, "%s: bstree_inorder_walk: root=%d, [", __FUNCTION__, tr->key);
		bstree_inorder_walk(tr);
		fprintf(stdout, "]\n");
	}

	return 0;
}

/* delete node with y is the right child of node*/
static int test_binary_search_tree_delete_4(int argc, char ** argv)
{
	bstree_node nodes[] = { {100}, {90}, {95}, {80}, {70}, {85}, {94}, {110}, {105}, {120},
			{103}, {108}, {115}, {118}, {119}, {130}, {140} };
	static size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bstree_node * tr;
	auto r = bstree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	int key = 120;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);

	if(n){
		fprintf(stdout, "%s: bstree_delete key=%d\n", __FUNCTION__, n->key);
		bstree_delete(tr, n);
		fprintf(stdout, "%s: bstree_inorder_walk: root=%d, [", __FUNCTION__, tr->key);
		bstree_inorder_walk(tr);
		fprintf(stdout, "]\n");
	}

	return 0;
}

static int bstree_init(bstree_node *& tr, bstree_node * nodes, size_t n)
{
	tr = NULL;
	for(size_t i = 0; i < n; ++i){
		if(bstree_insert(tr, &nodes[i]) != 0)
			return -1;
	}

	fprintf(stdout, "%s: bstree, init data: [", __FUNCTION__);
	for(size_t i = 0; i < n; ++i){
		fprintf(stdout, "%d, ", nodes[i].key);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: bstree_inorder_walk: root=%d, [", __FUNCTION__, tr->key);
	bstree_inorder_walk(tr);
	fprintf(stdout, "]\n");

	return 0;
}

int test_binary_search_tree_main(int argc, char ** argv)
{
	/* bstree visualization: http://www.cs.usfca.edu/~galles/visualization/BST.html */
	test_binary_search_tree_search(argc, argv);

	test_binary_search_tree_delete_1(argc, argv);
	test_binary_search_tree_delete_2(argc, argv);
	test_binary_search_tree_delete_3(argc, argv);
	test_binary_search_tree_delete_4(argc, argv);
	return 0;
}
