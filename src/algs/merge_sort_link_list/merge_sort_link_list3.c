/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/3
 *
 * merge_sort for link_list
 * code from https://stackoverflow.com/questions/7685/merge-sort-a-linked-list
 * */

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

typedef struct list_head list_head;

list_head * merge_sort_list(list_head *list,int (*compare)(list_head *one,list_head *two))
{
    int listSize=1,numMerges,leftSize,rightSize;
    list_head *tail,*left,*right,*next;
    if (!list || !list->next) return list;  // Trivial case

    do { // For each power of two<=list length
        numMerges=0,left=list;tail=list=0; // Start at the start

        while (left) { // Do this list_len/listSize times:
            numMerges++,right=left,leftSize=0,rightSize=listSize;
            // Cut list into two halves (but don't overrun)
            while (right && leftSize<listSize) leftSize++,right=right->next;
            // Run through the lists appending onto what we have so far.
            while (leftSize>0 || (rightSize>0 && right)) {
                // Left empty, take right OR Right empty, take left, OR compare.
                if (!leftSize)                  {next=right;right=right->next;rightSize--;}
                else if (!rightSize || !right)  {next=left;left=left->next;leftSize--;}
                else if (compare(left,right)<0) {next=left;left=left->next;leftSize--;}
                else                            {next=right;right=right->next;rightSize--;}
                // Update pointers to keep track of where we are:
                if (tail) tail->next=next;  else list=next;
                // Sort prev pointer
                next->prev=tail; // Optional.
                tail=next;
            }
            // Right is now AFTER the list we just sorted, so start the next sort there.
            left=right;
        }
        // Terminate the list, double the list-sort size.
        tail->next=0,listSize<<=1;
    } while (numMerges>1); // If we only did one merge, then we just sorted the whole list.
    return list;
}


typedef struct client_node {
	int              i;
	struct list_head list;
} CLIENT_NODE;

static int cmp(struct list_head *a, struct list_head *b)
{
	struct client_node * na = (struct client_node *)list_entry(a, struct client_node, list);
	struct client_node * nb = (struct client_node *)list_entry(b, struct client_node, list);
	return na->i - nb->i;
}

static struct client_node client_lists_obj, * client_lists = &client_lists_obj;
int merge_sort_link_list3_main(int argc, char ** argv)
{
	srandom(time(0));
	INIT_LIST_HEAD(&client_lists->list);

	int i;
	struct client_node nodes[10] = { 0 };
	for(i = 0; i < sizeof(nodes) / sizeof(nodes[10]); ++i){
		nodes[i].i = 1 + random() % 9;
		list_add_tail(&nodes[i].list, &client_lists->list);
	}

	fprintf(stdout, "before listsort:\n");

    struct list_head * pos, * next;
    list_for_each_safe(pos, next, &client_lists->list){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");

    /* FIXME: not work */
    merge_sort_list(&client_lists->list, cmp);

	fprintf(stdout, "after listsort:\n");

    list_for_each_safe(pos, next, &client_lists->list){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");
	return 0;
}

