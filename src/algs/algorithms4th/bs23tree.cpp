/*!
 * test 23tree, from book <Algorithms 4th Edition>_3.3.1
 * @author hongjun.liao <docici@126.com>, @date 2017//05/01
 *
 * 2-3查找树定义:
 *　(1)2-结点, 含有一个键(及其对应值)和两条链接, 左链接指向的2-3树中的键都小于该结点,
 *　   右链接指向的2-3树中的键都大于该结点;
 *　(2)3-结点, 含有两个键(及其对应值)和三条链接, 左链接指向的2-3树中的键都小于该结点,
 *　   中链接指向的2-3树中的键都位于该结点的两个键之间, 右链接指向的2-3树中的键都大于该结点
 *  NOTES:
 *  (1)2-结点和3-结点中的每条链接都对应着其中保存的键所分割产生的一个区间
 *  (2)局部变换: 将一个4-结点分解为一棵2-3树可能有6种情况, 这个4-结点可能是根结点,可能是一个2-结点的左子结点或右子结点,
 *     也可能是一个3-结点了左子结点, 中子结点或右子结点
 *  (3)全局性质: 这些局部变换不会影响树的全局有序性和平衡性: 任意空链接到根结点的简单路径长度都是相等的
 *　 a sample 23tree:
 *        _____M_____
 *       /           \
 *    __EJ_         __R__
 *   /  |  \       /     \
 *  AC  H   L     P      SX
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <limits.h>	/* INT_MAX */

struct bs23tree_node {
	bs23tree_node * left;     /* left child */
	bs23tree_node * midle;    /* midle child */
	bs23tree_node * right;    /* right child */
	bs23tree_node * p;	      /* parent */

	int lkey, rkey;
};

#define BS23TR_NODE(key)       (bs23tree_node{0, 0, 0, 0, key, -INT_MAX})
#define BS23TR_IS_3NODE(node)  ((node)->rkey != -INT_MAX)


/* NOTE: @param node.rkey ignored
 * @return return @param node or an existing node
 *
 * FIXME: this function has bugs and only for test only!
 * */
bs23tree_node * bs23tree_insert(bs23tree_node *& root, bs23tree_node * node)
{
	if(!node)
		return 0;

	bs23tree_node * p = 0;
	for(auto x = root; x; ){
		p = x;

		auto & key = node->lkey;

		if(!x->p || BS23TR_IS_3NODE(x)){	/* 3-node, put to child */
			if(x->p && BS23TR_IS_3NODE(x) && (key > x->lkey && key < x->rkey))
				x = x->midle;
			else if(key < x->lkey)
				x = x->left;
			else
				x = x->right;
		}
		else{ /* 2-node, change to 3-node */
			if(key < x->lkey){
				x->rkey = x->lkey;
				x->lkey = key;
			}
			else
				x->rkey = key;
			return x;	/* no new node inserted */
		}
	}
	if(!p)
		root = node;
	else{
		auto & key = node->lkey;
		if(p->p && BS23TR_IS_3NODE(p) && (key > p->lkey && key < p->rkey ))
			p->midle = node;
		else if(key < p->lkey)
			p->left = node;
		else
			p->right = node;
	}

	node->p = p;

	return node;
}

void bs23tree_inorder_walk(bs23tree_node const * root)
{
	fprintf(stdout, "%s: NOT implement yet!\n", __FUNCTION__);
	if(!root)
		return;
}

bs23tree_node * bs23tree_search(bs23tree_node *& root, int key)
{
	auto x = root;
	for(; x && (x->lkey != key || x->p && BS23TR_IS_3NODE(x) && x->rkey != key); ){
		if(x->p && BS23TR_IS_3NODE(x) && key > x->lkey && key < x->rkey)
			x = x->midle;
		else if(key < x->lkey)
			x = x->left;
		else
			x = x->right;
	}
	return x;
}

static int bs23tree_init(bs23tree_node *& tr, bs23tree_node * nodes, size_t n)
{
	fprintf(stdout, "%s: size=%zu\n", __FUNCTION__, n);

	tr = NULL;
	for(size_t i = 0; i < n; ++i){
		if(!bs23tree_insert(tr, &nodes[i])){
			fprintf(stderr, "%s: insert node=%c failed\n", __FUNCTION__, nodes[i].lkey);
			return -1;
		}
	}

	fprintf(stdout, "%s: data: [", __FUNCTION__);
	for(size_t i = 0; i < n; ++i){
		fprintf(stdout, "%c,", nodes[i].lkey);
	}
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: root=%c, [", __FUNCTION__, tr->lkey);
	bs23tree_inorder_walk(tr);
	fprintf(stdout, "]\n");

	return 0;
}

int test_bs23tree_main(int argc, char ** argv)
{
	bs23tree_node nodes[] = { BS23TR_NODE('M'), BS23TR_NODE('E'), BS23TR_NODE('J'), BS23TR_NODE('A'), BS23TR_NODE('C'),
			BS23TR_NODE('H'), BS23TR_NODE('L'), BS23TR_NODE('R'), BS23TR_NODE('P'), BS23TR_NODE('S'), BS23TR_NODE('X') };
	size_t NODE_SZ = sizeof(nodes) / sizeof(nodes[0]);

	bs23tree_node * tr = 0;
	auto r = bs23tree_init(tr, nodes, NODE_SZ);
	if(!tr || r != 0) return -1;

	return -1;
}
