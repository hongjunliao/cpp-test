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

/* print bstree to terminal, like this:
 *      _____H______
 *  ____C___    ____R_____
 *  A      E __M__      __X
 *           L    P    S
 */
void rbtree_draw_to_term(rb_tree const& tr)
{
	fprintf(stdout, "%s: \n", __FUNCTION__);

    int curLayerCount = 0; 	//当前层中的节点数
    int nextLayerCount = 0; //下一层中的节点数

    std::queue<rbtree_node *> nodes;
    nodes.push(tr.root);

    std::vector<rbtree_node * > vec[30];
    int n = 0;

    curLayerCount++;
    while(!nodes.empty()){
    	auto p = nodes.front();
    	nodes.pop();
        curLayerCount--;

        vec[n].push_back(p);

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

            ++n;
        }
    }
    /* find layer where nodes count max */
    auto it = std::max_element(std::begin(vec), std::end(vec), comp_by_vec_size);
    if(it == std::end(vec))
    	return;

    int nchars = it->size() * 2;	/* every node at least 3 chars */
    fprintf(stdout, "%s: n=%d, max_nodes=%zu, nchars=%d\n", __FUNCTION__, n, it->size(), nchars);
    for(int i = 0; i < n; ++i){

    	char line[1024 * 5];
    	line[0] = '\0';

    	int nc = nchars / vec[i].size() / 2 ;
    	for(auto & node : vec[i]){
    		char s[1024];
    		std::string pad(nc, '_'), pad2 = pad;
    		pad2.insert(0, std::string(it->size() - vec[i].size() + 1, ' '));
    		sprintf(s, "%s%c%s", pad2.c_str(), node->key, pad.c_str());
    		strcat(line, s);

//    		fprintf(stdout, "%s: padding='%s', fmt='%s', s='%s'\n", __FUNCTION__, pad.c_str(), fmt, s);
    	}

    	printf("%s\n", line);
    }
}
