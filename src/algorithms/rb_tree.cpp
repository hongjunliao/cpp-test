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
struct rbtree_node {
	rbtree_node * left;     /* left child */
	rbtree_node * right;    /* right child */
	rbtree_node * p;	    /* parent */

	bool red;				/* true if read, else black */

	int key;
};

/* left_rotate
 * NOTES: @param node MUST have right_child
 * @return 0 on success
 * */
static int rbtree_left_rotate(rbtree_node *& root, rbtree_node * node);


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
static int rbtree_left_rotate(rbtree_node *& root, rbtree_node * x)
{
	if(!x || !x->right)
		return -1;

	auto y = x->right;
	/* turn y's left to x's right */
	x->right = y->left;
	if(y->left)
		y->left->p = x;
	/* linkx x'parent to y */
	y->p = x->p;
	if(!x->p)
		root = y;
	else if(x == x->p->left)
		x->p->left = y;
	else
		x->p->right = y;
	/* put x on y's left */
	y->left = x;
	x->p = y;
}


int test_rbtree_main(int argc, char ** argv)
{
	return -1;
}
