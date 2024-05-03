/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2018/5/3
 *
 * merge_sort for link_list
 * */

#include "hp/klist.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>

#define FALSE 0
#define TRUE 1

typedef struct list_head element;

static int cmp(element *a, element *b);

/*
 * This is the actual sort function. Notice that it returns the new
 * head of the list. (It has to, because the head will not
 * generally be the same element after the sort.) So unlike sorting
 * an array, where you can do
 *
 *     sort(myarray);
 *
 * you now have to do
 *
 *     list = listsort(mylist);
 */
static element *listsort(element *list, int is_circular, int is_double)
{
	element *p, *q, *e, *tail, *oldhead;
	int insize, nmerges, psize, qsize, i;

	/*
	 * Silly special case: if `list' was passed in as NULL, return
	 * NULL immediately.
	 */
	if (!list)
		return NULL;

	insize = 1;

	while (1) {
		p = list;
		oldhead = list; /* only used for circular linkage */
		list = NULL;
		tail = NULL;

		nmerges = 0; /* count number of merges we do in this pass */

		while (p) {
			nmerges++; /* there exists a merge to be done */
			/* step `insize' places along from p */
			q = p;
			psize = 0;
			for (i = 0; i < insize; i++) {
				psize++;
				if (is_circular)
					q = (q->next == oldhead ? NULL : q->next);
				else
					q = q->next;
				if (!q)
					break;
			}

			/* if q hasn't fallen off end, we have two lists to merge */
			qsize = insize;

			/* now we have two lists; merge them */
			while (psize > 0 || (qsize > 0 && q)) {

				/* decide whether next element of merge comes from p or q */
				if (psize == 0) {
					/* p is empty; e must come from q. */
					e = q;
					q = q->next;
					qsize--;
					if (is_circular && q == oldhead)
						q = NULL;
				} else if (qsize == 0 || !q) {
					/* q is empty; e must come from p. */
					e = p;
					p = p->next;
					psize--;
					if (is_circular && p == oldhead)
						p = NULL;
				} else if (cmp(p, q) <= 0) {
					/* First element of p is lower (or same);
					 * e must come from p. */
					e = p;
					p = p->next;
					psize--;
					if (is_circular && p == oldhead)
						p = NULL;
				} else {
					/* First element of q is lower; e must come from q. */
					e = q;
					q = q->next;
					qsize--;
					if (is_circular && q == oldhead)
						q = NULL;
				}

				/* add the next element to the merged list */
				if (tail) {
					tail->next = e;
				} else {
					list = e;
				}
				if (is_double) {
					/* Maintain reverse pointers in a doubly linked list. */
					e->prev = tail;
				}
				tail = e;
			}

			/* now p has stepped `insize' places along, and q has too */
			p = q;
		}
		if (is_circular) {
			tail->next = list;
			if (is_double)
				list->prev = tail;
		} else
			tail->next = NULL;

		/* If we have done only one merge, we're finished. */
		if (nmerges <= 1) /* allow for nmerges==0, the empty list case */
			return list;

		/* Otherwise repeat, merging lists twice the size */
		insize *= 2;
	}
}

typedef struct client_node {
	int              i;
	struct list_head list;
} CLIENT_NODE;

static int cmp(element *a, element *b)
{
	struct client_node * na = (struct client_node *)list_entry(a, struct client_node, list);
	struct client_node * nb = (struct client_node *)list_entry(b, struct client_node, list);
	return na->i - nb->i;
}

static struct list_head client_lists_obj, * client_lists = &client_lists_obj;
int merge_sort_link_list2_main(int argc, char ** argv)
{
	srandom(time(0));
	INIT_LIST_HEAD(client_lists);

	int i;
	struct client_node nodes[10] = { 0 };
	for(i = 0; i < sizeof(nodes) / sizeof(nodes[10]); ++i){
		nodes[i].i = 1 + random() % 9;
		list_add_tail(&nodes[i].list, client_lists);
	}

	fprintf(stdout, "before listsort:\n");

    struct list_head * pos, * next;
    list_for_each_safe(pos, next, client_lists){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");

	listsort(client_lists, 1, 1);

	fprintf(stdout, "after listsort:\n");

    list_for_each_safe(pos, next, client_lists){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");
	return 0;
}
