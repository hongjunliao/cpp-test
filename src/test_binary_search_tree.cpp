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

struct btree_node {
	int key;
	btree_node * left;  /* left child */
	btree_node * right; /* right child */

	btree_node * p;	    /* parent */
};

btree_node const * btree_max(btree_node const * root)
{
	while(root->right)
		root = root->right;
	return root;
}

btree_node const * btree_min(btree_node const * root)
{
	while(root->left)
		root = root->left;
	return root;
}

/* @param root maybe null, in this condition, @param node becomes the root
 * @return: 0 on success */
int btree_insert(btree_node *& root, btree_node * node)
{
	if(!node)
		return -1;

	btree_node * p = 0;
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

btree_node const * btree_search(btree_node const * root, int key)
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
btree_node const * btree_search_recursive(btree_node const * root, int key)
{
	if(!root || key == root->key)
		return root;

	if(key <= root->key)
		return btree_search(root->left, key);

	return btree_search(root->right, key);
}

int btree_delete(btree_node const * root, btree_node const * node)
{
	return -1;
	if(!node)
		return -1;

	if(!node->left && !node->right){	/* no child */
		auto & n = node->p->left == node? node->p->left : node->p->right;
		n = 0;
		return 0;
	}
	/* both 2 childs */
	if(node->left && node->right){
		return 0;
	}

	/* one child */
	auto & n = node->p->left == node? node->p->left : node->p->right;
	n = node->left? node->left : node->right;

	return 0;
}

/* verify if  @param root is a bst */
int btree_debug_verrify(btree_node const * root)
{

}

void btree_inorder_walk(btree_node const * root)
{
	if(!root)
		return;

	btree_inorder_walk(root->left);
	fprintf(stdout, "%d, ", root->key);
	btree_inorder_walk(root->right);
}

static int test_binary_search_tree_main_1(int argc, char ** argv)
{
	int NODE_SZ = 10;
	btree_node nodes[NODE_SZ];

	btree_node * tr = NULL;

	fprintf(stdout, "%s: btree, init data: [", __FUNCTION__);
	for(int i = 0; i < NODE_SZ; ++i){
		nodes[i].left = nodes[i].right = nodes[i].p = 0;
		nodes[i].key = rand() % 20;

		fprintf(stdout, "%d, ", nodes[i].key);

		btree_insert(tr, &nodes[i]);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: btree_inorder_walk: root=%d\n", __FUNCTION__, tr->key);
	btree_inorder_walk(tr);
	fprintf(stdout, "\n");


	auto maxn = btree_max(tr);
	fprintf(stdout, "%s: btree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = btree_min(tr);
	fprintf(stdout, "%s: btree_min=%d\n", __FUNCTION__, minn->key);

	int key = 12;
	auto n = btree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: btree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: btree_search, key=%d, NOT found\n", __FUNCTION__, key);


	auto result = btree_delete(tr, n);

}

static int test_binary_search_tree_main_2(int argc, char ** argv)
{
	int NODE_SZ = 10;

	/* init bst with array */
	btree_node * tr = NULL;
	btree_node nodes[] = { {7}, {11}, {4}, {8}, {12}, {15}, {11}, {11}, {13}, {13} };
	for(auto & n : nodes){
		if(btree_insert(tr, &n) != 0)
			return -1;
	}

	fprintf(stdout, "%s: btree, init data: [", __FUNCTION__);
	for(int i = 0; i < NODE_SZ; ++i){
		fprintf(stdout, "%d, ", nodes[i].key);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: btree_inorder_walk: root=%d\n", __FUNCTION__, tr->key);
	btree_inorder_walk(tr);
	fprintf(stdout, "\n");


	auto maxn = btree_max(tr);
	fprintf(stdout, "%s: btree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = btree_min(tr);
	fprintf(stdout, "%s: btree_min=%d\n", __FUNCTION__, minn->key);

	int key = 12;
	auto n = btree_search(tr, key);
	if(n)
		fprintf(stdout, "%s: btree_search, key=%d, left=%p(%d), right=%p(%d), parent=%p(%d)\n", __FUNCTION__,
				key, n->left, n->left? n->left->key : -1, n->right, n->right? n->right->key : -1,
				n->p, n->p? n->p->key : -1);
	else
		fprintf(stdout, "%s: btree_search, key=%d, NOT found\n", __FUNCTION__, key);


	auto result = btree_delete(tr, n);

}

int test_binary_search_tree_main(int argc, char ** argv)
{
	srand(time(0));

	test_binary_search_tree_main_1(argc, argv);
	test_binary_search_tree_main_2(argc, argv);

	return 0;
}
