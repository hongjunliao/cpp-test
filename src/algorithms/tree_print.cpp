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

        char buf[512];
        if(!p->p)
        	fprintf(stdout, "%s", tr.node_c_str(p->data, buf, sizeof(buf)));
        else if(p->p->left == p)
        	fprintf(stdout, "%s(%s) ", tr.node_c_str(p->data, buf, sizeof(buf)),
        			tr.node_c_str(p->p->data, buf, sizeof(buf)));
        else if(p->p->right == p)
        	fprintf(stdout, "(%s)%s ", tr.node_c_str(p->p->data, buf, sizeof(buf)),
        			tr.node_c_str(p->data, buf, sizeof(buf)));
        else
        	fprintf(stdout, "%s", tr.node_c_str(p->data, buf, sizeof(buf)));

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
