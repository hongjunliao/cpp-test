/*!
 * test 23tree, from book <Algorithms 4th Edition>_3.3.1
 * @author hongjun.liao <docici@126.com>, @date 2017//05/01
 *
 * 2-3查找树定义:
 *　(1)2-结点, 含有一个键(及其对应值)和两条链接, 左链接指向的2-3树中的键都小于该结点,
 *　   右链接指向的2-3树中的键都大于该结点;
 *　(2)3-结点, 含有两个键(及其对应值)和三条链接, 左链接指向的2-3树中的键都小于该结点,
 *　   中链接指向的2-3树中的键都位于该结点的两个键之间, 右链接指向的2-3树中的键都大于该结点
 *
 *　 a sample 23tree:
 *        _____M_____
 *       /           \
 *    __EJ_         __R__
 *   /  |  \       /     \
 *  AC  H   L     P      SX
 */

#include <stdio.h>

struct bs23tree_node {
	bs23tree_node * left;     /* left child */
	bs23tree_node * midle;    /* midle child */
	bs23tree_node * right;    /* right child */
	bs23tree_node * p;	      /* parent */

	int lkey, rkey;           /* if midle NULL, rkey invalid */
};

#define BS23TR_KEY(key)     (bs23tree_node{0, 0, 0, 0, key, 0})
#define BS23TR_KEY2(lrkey)    (bs23tree_node{0, 0, 0, 0, lrkey[0], lrkey[1]})

/* NOTE: @param node.rkey ignored
 * @return 0 on success */
int bs23tree_insert(bs23tree_node *& root, bs23tree_node * node)
{
	if(!node)
		return -1;

	bs23tree_node * p = 0;
	for(auto x = root; x; ){
		p = x;

		auto & key = x->lkey;
		if(x->midle && (key > x->lkey && key < x->rkey))
			x = x->midle;
		else if(key < node->lkey)
			x = x->left;
		else
			x = x->right;
	}
	if(!p)
		root = node;
	else if(p->midle && (node->lkey > p->lkey && node->lkey < p->rkey ))
		p->midle = node;
	else if(node->lkey < p->lkey)
		p->left = node;
	else
		p->right = node;

	node->p = p;

	return 0;
}

static int bs23tree_init(bs23tree_node *& tr, bs23tree_node * nodes, size_t n)
{
	tr = NULL;
	for(size_t i = 0; i < n; ++i){
		if(bs23tree_insert(tr, &nodes[i]) != 0)
			return -1;
	}

	fprintf(stdout, "%s: data: [", __FUNCTION__);
	for(size_t i = 0; i < n; ++i){
		fprintf(stdout, "%c", nodes[i].lkey);
	}
	fprintf(stdout, "]\n");

	return 0;
}

int test_bs23tree_main(int argc, char ** argv)
{
	bs23tree_node nodes[] = { BS23TR_KEY('M'), BS23TR_KEY2("EJ"), BS23TR_KEY2("AC"),
			BS23TR_KEY('H'), BS23TR_KEY('L'), BS23TR_KEY('R'), BS23TR_KEY('P'), BS23TR_KEY2("SX") };
	size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bs23tree_node * tr = 0;
	auto r = bs23tree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	return -1;
}
