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

struct btree_node {
	int key;
	btree_node * left;
	btree_node * right;
};

btree_node const * btree_max(btree_node const& root)
{
	if(root.right)
		return btree_max(*root.right);
	return &root;
}

btree_node const * btree_min(btree_node const& root)
{
	if(root.left)
		return btree_max(*root.left);
	return &root;
}

void btree_insert(btree_node & root, btree_node * node)
{
	if(node->key < root.key){
		if(root.left)
			return btree_insert(*root.left, node);
		else
			root.left = node;
	}
	else{
		if(root.right)
			return btree_insert(*root.right, node);
		else
			root.right = node;
	}
}

btree_node const * btree_search(btree_node const& root, int key)
{

}

/* verify if  @param root is a bst */
int btree_debug_verrify(btree_node const& root)
{

}

void btree_inorder_walk(btree_node const& root)
{
	if(root.left)
		btree_inorder_walk(*root.left);

	fprintf(stdout, "%d, ", root.key);

	if(root.right)
		btree_inorder_walk(*root.right);
}

int test_binary_search_tree_main(int argc, char ** argv)
{
	srand(time(0));

	int NODE_SZ = 50;
	btree_node nodes[NODE_SZ];

	btree_node tr { rand() % 50, 0, 0};

	for(int i = 0; i < NODE_SZ; ++i){
		nodes[i].left = nodes[i].right = 0;
		nodes[i].key = rand() % 50;

		btree_insert(tr, &nodes[i]);
	}

	fprintf(stdout, "%s: btree_inorder_walk: \n", __FUNCTION__);
	btree_inorder_walk(tr);
	fprintf(stdout, "\n");


	auto maxn = btree_max(tr);
	fprintf(stdout, "%s: btree_max=%d\n", __FUNCTION__, maxn->key);

	auto minn = btree_min(tr);
	fprintf(stdout, "%s: btree_min=%d\n", __FUNCTION__, minn->key);

	auto n = btree_search(tr, 20);


}
