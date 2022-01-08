/*!
 * This file is PART of docici/cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2017//05/31
 * from <剑指offer>_2.7, 两个栈实现队列
 */
#if 0
#include <stdio.h>
#include <stdlib.h>    /* realloc */
#include <string.h>    /* memmove */
#include <math.h>      /* log2 */
#include <stack>       /* std::stack */

struct stk_queue{
	int * s1;
	size_t i1;

	int * s2;
	size_t i2;

	static size_t sz;
};

static void stk_queue_resize(stk_queue & q, size_t sz)
{
	q.sz = sz;
	q.s1 = (int *)realloc(q.s1, sz * sizeof(int));
	q.s2 = (int *)realloc(q.s2, sz * sizeof(int));
	q.i1 = q.i2 = 0;
}

void stk_queue_init(stk_queue & q, size_t sz)
{
	q.s1 = q.s2 = 0;
	stk_queue_resize(q, sz);
}

bool stk_queue_empty(stk_queue const& q)
{
	return q.i1 == 0;
}

int stk_queue_front(stk_queue const& q)
{
	return q.s1[0];
}

int stk_queue_back(stk_queue const& q)
{
	return q.s1[q.i1];
}

void stk_queue_push(stk_queue & q, int val)
{
	if(q.i1 == q.sz - 1){
		size_t n = log(q.sz) < 8? 8 : log2(q.sz);
		stk_queue_resize(q, q.sz + n);
	}
	q.s1[q.i1++] = val;

	q.i2 = 0;
	for(auto i = q.i1; i >= 0; --i)
		q.s2[q.i2++] = q.s1[i];

}

void stk_queue_pop(stk_queue & q)
{
	--q.i2;
	memmove(q.s1 + 1, q.s1, q.i1 * sizeof(int));
}

/* TODO:　test me! */
int test_stack_queue_main(int argc, char ** argv)
{
	stk_queue q;
	stk_queue_init(q, 50);
	for(int i = 0; i < 50; ++i)
		stk_queue_push(q, i);
	auto f = stk_queue_front(q);
	auto b = stk_queue_back(q);
	stk_queue_pop(q);

	f = stk_queue_front(q);

	return 0;
}

#endif
