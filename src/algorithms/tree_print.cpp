/*!
 * test rb_tree/red_black_tree print to terminal
 * @author hongjun.liao <docici@126.com>, @date 2017//05/04
 */

#include "rb_tree.h"  /* rb_tree */
#include <string.h>	  /* strcpy */

#include <string>	  /* std::string */
#include <queue>	  /* std::queue */
#include <vector>	  /* std::vector */
#include <algorithm>  /* std::max_element */

static bool comp_by_vec_size(std::vector<rbtree_node * > const& a,
		std::vector<rbtree_node * > const& b)
{
	return a.size() < b.size();
}

static int bstree_left_nodes(rbtree_node const * node)
{
	if(!node)
		return 0;

	int n = 0;
	while(node->left){
		++n;
		node = node->left;
	}
	return n;
}

/* print bstree to terminal, like this:
 * H
 * C(H), (H)R
 * A(C), (C)E, M(R), (R)X
 * L(M), (M)P, S(X)
 */
void rbtree_draw_to_term(rb_tree const& tr)
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
