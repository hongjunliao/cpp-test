/*!
 * This file is PART of docici/cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2017//05/31
 * from <剑指offer>_2.6, 重建二叉树
 */

#include <stdio.h>
#include <stdlib.h>    /* malloc */
#include <algorithm>   /* std::find */

struct btr_node {
	int key;
	btr_node * left;
	btr_node * right;
};

static btr_node * gnodes = 0;
static size_t ginodes = 0;

static btr_node * btrnode_get()
{
	size_t N = 1024 * 1024 / sizeof(btr_node);
	for(;;){
		 if(!gnodes){
			 gnodes = (btr_node *)malloc(N * sizeof(btr_node));
			 if(!gnodes){
				 fprintf(stderr, "%s: malloc failed\n", __FUNCTION__);
				 exit(0);
			 }
		 }
		 /* FIXME: memory leak!!! ugly but worked */
		 if(ginodes == N - 1){
			 gnodes = 0;
			 ginodes = 0;
			 continue;
		 }
		 return &gnodes[ginodes++];
	}
}

static void btr_inorder_print(btr_node *& tr)
{
	if(!tr)
		return;
	btr_inorder_print(tr->left);
	fprintf(stdout, "%d, ", tr->key);
	btr_inorder_print(tr->right);
}

/*
 * 重建二叉树, 示例:
 * 输入二叉树的前序遍历结果: {1, 2, 4, 7, 3, 5, 6, 8}
 * 和中序遍历结果: {4, 7, 2, 1, 5, 3, 8, 6}
 * 重建该二叉树:
 *         __1__
 *      __2   __3__
 *     4_    5   __6
 *       7      8
 * @param tr: root node for btr
 * @param pre: pre_order
 * @param in:: in_order
 * @param len: length for @param pre and @param in
 * @return: 0 on success
 *  */
int construct_btr_pre_in(btr_node *& tr, int * pre, int * in, size_t len)
{
	if(!(pre && in && len > 0))
		return -2;

	int root = pre[0];
	int * it = std::find(in, in + len, root);
	if(!it)
		return -1;

	tr = btrnode_get();
	if(!tr)
		return -3;
	tr->key = root;

	size_t leftn = it - in;
	if(leftn > 0){
		int r = construct_btr_pre_in(tr->left, pre + 1, in, leftn);
		if(r != 0)
			return -4;
	}
	else
		tr->left = 0;

	size_t rightn = len - leftn - 1;
	if(rightn > 0){
		int r = construct_btr_pre_in(tr->right, pre + 1 + leftn, in + 1 + leftn, rightn);
		if(r != 0)
			return -5;
	}
	else
		tr->right = 0;

	return 0;
}

int test_jzoffter_construct_btr_main(int argc, char ** argv)
{
	int pre[] = {1, 2, 4, 7, 3, 5, 6, 8};
	int in[] = {4, 7, 2, 1, 5, 3, 8, 6};
	size_t const N = sizeof(pre) / sizeof(pre[0]);

	btr_node * tr = 0;
	int r = construct_btr_pre_in(tr, pre, in, N);
	if(!(tr && r == 0)){
		fprintf(stderr, "%s: construct_btr_pre_in failed, tr=%p,r=%d\n", __FUNCTION__, tr, r);
		return -1;
	}

	fprintf(stdout, "%s: inorder=[", __FUNCTION__);
	btr_inorder_print(tr);
	fprintf(stdout, "]\n");

	return 0;
}
