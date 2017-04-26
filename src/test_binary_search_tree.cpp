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

bstree_node const * bstree_max(bstree_node const * root)
{
	while(root->right)
		root = root->right;
	return root;
}

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

	auto & n = u->p->left == u? u->p->left : u->p->right;
	n = v;

	if(v)
		v->p = u->p;
}

/* TODO: test me */
int bstree_delete(bstree_node *& root, bstree_node const * node)
{
	if(!node)
		return -1;

	if(!node->left){	/* no left_child, replace with right_child(maybe null) */
		bstree_transplant(root, node, node->right);
		return 0;
	}
	if(node->left && !node->right){ /* only left_child, replace with left_child */
		bstree_transplant(root, node, node->left);
		return 0;
	}

	auto y = bstree_min(node->right);
	if(y->p != node){
		bstree_transplant(root, y, y->right);
		y->right = node->right;
		y->right->p = y;
	}
	bstree_transplant(root, node, y);
	y->left = node->left;
	y->left->p = y;

	return 0;
}

/* verify if  @param root is a bst */
int bstree_debug_verrify(bstree_node const * root)
{

}

void bstree_inorder_walk(bstree_node const * root)
{
	if(!root)
		return;

	bstree_inorder_walk(root->left);
	fprintf(stdout, "%d, ", root->key);
	bstree_inorder_walk(root->right);
}

static int test_binary_search_tree_main_1(int argc, char ** argv)
{
	int NODE_SZ = 10;
	bstree_node nodes[NODE_SZ];

	bstree_node * tr = NULL;

	fprintf(stdout, "%s: bstree, init data: [", __FUNCTION__);
	for(int i = 0; i < NODE_SZ; ++i){
		nodes[i].left = nodes[i].right = nodes[i].p = 0;
		nodes[i].key = rand() % 20;

		fprintf(stdout, "%d, ", nodes[i].key);

		bstree_insert(tr, &nodes[i]);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: bstree_inorder_walk: root=%d\n", __FUNCTION__, tr->key);
	bstree_inorder_walk(tr);
	fprintf(stdout, "\n");


	auto maxn = bstree_max(tr);
	fprintf(stdout, "%s: bstree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = bstree_min(tr);
	fprintf(stdout, "%s: bstree_min=%d\n", __FUNCTION__, minn->key);

	int key = 12;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);


	auto result = bstree_delete(tr, n);

}

static int test_binary_search_tree_main_2(int argc, char ** argv)
{
	int NODE_SZ = 10;

	/* init bst with array */
	bstree_node * tr = NULL;
	bstree_node nodes[] = { {7}, {11}, {4}, {8}, {12}, {15}, {11}, {11}, {13}, {13} };
	for(auto & n : nodes){
		if(bstree_insert(tr, &n) != 0)
			return -1;
	}

	fprintf(stdout, "%s: bstree, init data: [", __FUNCTION__);
	for(int i = 0; i < NODE_SZ; ++i){
		fprintf(stdout, "%d, ", nodes[i].key);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: bstree_inorder_walk: root=%d\n", __FUNCTION__, tr->key);
	bstree_inorder_walk(tr);
	fprintf(stdout, "\n");


	auto maxn = bstree_max(tr);
	fprintf(stdout, "%s: bstree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = bstree_min(tr);
	fprintf(stdout, "%s: bstree_min=%d\n", __FUNCTION__, minn->key);

	int key = 12;
	auto n = bstree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: bstree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: bstree_search, key=%d, NOT found\n", __FUNCTION__, key);


	auto result = bstree_delete(tr, n);

}

int test_binary_search_tree_main(int argc, char ** argv)
{
	srand(time(0));

	test_binary_search_tree_main_1(argc, argv);
	test_binary_search_tree_main_2(argc, argv);

	return 0;
}
