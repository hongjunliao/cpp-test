/**!
 * the main source code is from:
 * from https://github.com/torvalds/linux/blob/master/lib/list_sort.c
 */

// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#ifndef CP_NO_KERNEL_LIST_SORT
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define MAX_LIST_LENGTH_BITS 20

/*
 * Returns a list organized in an intermediate format suited
 * to chaining of merge() calls: null-terminated, no reserved or
 * sentinel head node, "prev" links not maintained.
 */
static struct list_head *merge(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *a, struct list_head *b)
{
	struct list_head head, *tail = &head;

	while (a && b) {
		/* if equal, take 'a' -- important for sort stability */
		if ((*cmp)(priv, a, b) <= 0) {
			tail->next = a;
			a = a->next;
		} else {
			tail->next = b;
			b = b->next;
		}
		tail = tail->next;
	}
	tail->next = a?:b;
	return head.next;
}

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
static void merge_and_restore_back_links(void *priv,
				int (*cmp)(void *priv, struct list_head *a,
					struct list_head *b),
				struct list_head *head,
				struct list_head *a, struct list_head *b)
{
	struct list_head *tail = head;

	while (a && b) {
		/* if equal, take 'a' -- important for sort stability */
		if ((*cmp)(priv, a, b) <= 0) {
			tail->next = a;
			a->prev = tail;
			a = a->next;
		} else {
			tail->next = b;
			b->prev = tail;
			b = b->next;
		}
		tail = tail->next;
	}
	tail->next = a ? : b;

	do {
		/*
		 * In worst cases this loop may run many iterations.
		 * Continue callbacks to the client even though no
		 * element comparison is needed, so the client's cmp()
		 * routine can invoke cond_resched() periodically.
		 */
		(*cmp)(priv, tail->next, tail->next);

		tail->next->prev = tail;
		tail = tail->next;
	} while (tail->next);

	tail->next = head;
	head->prev = tail;
}

/**
 * list_sort - sort a list
 * @priv: private data, opaque to list_sort(), passed to @cmp
 * @head: the list to sort
 * @cmp: the elements comparison function
 *
 * This function implements "merge sort", which has O(nlog(n))
 * complexity.
 *
 * The comparison function @cmp must return a negative value if @a
 * should sort before @b, and a positive value if @a should sort after
 * @b. If @a and @b are equivalent, and their original relative
 * ordering is to be preserved, @cmp must return 0.
 */
void list_sort(void *priv, struct list_head *head,
		int (*cmp)(void *priv, struct list_head *a,
			struct list_head *b))
{
	struct list_head *part[MAX_LIST_LENGTH_BITS+1]; /* sorted partial lists
						-- last slot is a sentinel */
	int lev;  /* index into part[] */
	int max_lev = 0;
	struct list_head *list;

	if (list_empty(head))
		return;

	memset(part, 0, sizeof(part));

	head->prev->next = NULL;
	list = head->next;

	while (list) {
		struct list_head *cur = list;
		list = list->next;
		cur->next = NULL;

		for (lev = 0; part[lev]; lev++) {
			cur = merge(priv, cmp, part[lev], cur);
			part[lev] = NULL;
		}
		if (lev > max_lev) {
			if ((lev >= ARRAY_SIZE(part)-1)) {
				fprintf(stderr, "%s: list too long for efficiency\n", __FUNCTION__);
				lev--;
			}
			max_lev = lev;
		}
		part[lev] = cur;
	}

	for (lev = 0; lev < max_lev; lev++)
		if (part[lev])
			list = merge(priv, cmp, part[lev], list);

	merge_and_restore_back_links(priv, cmp, head, part[max_lev], list);
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
#include <time.h>
#include <assert.h>

typedef struct client_node {
	int              i;
	struct list_head list;
} CLIENT_NODE;

static int cmp(void * priv, struct list_head *a, struct list_head *b)
{
	struct client_node * na = (struct client_node *)list_entry(a, struct client_node, list);
	struct client_node * nb = (struct client_node *)list_entry(b, struct client_node, list);
	return na->i - nb->i;
}

static struct list_head client_lists_obj, * client_lists = &client_lists_obj;
int test_kernel_list_sort_main(int argc, char ** argv)
{
	srandom(time(0));
	INIT_LIST_HEAD(client_lists);

	int i;
	struct client_node nodes[10] = { 0 };
	for(i = 0; i < sizeof(nodes) / sizeof(nodes[10]); ++i){
		nodes[i].i = 1 + random() % 9;
		list_add_tail(&nodes[i].list, client_lists);
	}

	fprintf(stdout, "before sort:\n");

    struct list_head * pos, * next;
    list_for_each_safe(pos, next, client_lists){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");

	list_sort(0, client_lists, cmp);

	fprintf(stdout, "after sort:\n");

    list_for_each_safe(pos, next, client_lists){
    	struct client_node * node = (struct client_node *)list_entry(pos, CLIENT_NODE, list);
    	fprintf(stdout, "%d, ", node->i);
    }
    fprintf(stdout, "\n");
	return 0;
}

#else
int test_kernel_list_sort_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: undefine CP_NO_KERNEL_LIST_SORT to enable this test\n", __FUNCTION__);
	return -1;
}
#endif /* NDEBUG */
#endif /* CP_NO_KERNEL_LIST_SORT */
