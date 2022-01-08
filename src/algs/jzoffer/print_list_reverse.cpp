/*!
 * This file is PART of docici/cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2017//05/31
 * from <剑指offer>_2.5, 反向打印单链表
 */
#include <stdio.h>
#include <stack>      /* std::stack */

struct listnode {
	int key;
	listnode * next;
};

/* use stack */
static void print_list_reverse_1(listnode * li, FILE * f)
{
	std::stack<listnode *> s;
	for(listnode * p = li; p; p = p->next)
		s.push(p);
	while(!s.empty()){
		listnode * p = s.top();
		s.pop();
		fprintf(f, "%d, ", p->key);
	}
}

/* use recursive */
static void print_list_reverse_2(listnode * li, FILE * f)
{
	if(li->next)
		print_list_reverse_2(li->next, f);
	fprintf(f, "%d, ", li->key);
}

int test_jzoffter_print_list_reverse_main(int argc, char ** argv)
{
	size_t N = 32;
	listnode nodes[N];

	listnode * li = 0;
	for(size_t i = 0; i < N; ++i){
		if(!li){
			li = &nodes[i];
			li->key = i;
			li->next = 0;
			continue;
		}
		listnode * node = &nodes[i];
		node->key = i;
		node->next = li;

		li = node;
	}

	fprintf(stdout, "%s: list=[", __FUNCTION__);
	for(listnode * p = li; p; p = p->next)
		fprintf(stdout, "%d, ", p->key);
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: reverse print list(stack)=[", __FUNCTION__);
	print_list_reverse_1(li, stdout);
	fprintf(stdout, "]\n");

	fprintf(stdout, "%s: reverse print list(recursive)=[", __FUNCTION__);
	print_list_reverse_2(li, stdout);
	fprintf(stdout, "]\n");

	return 0;
}
