/*!
 * a simple memory pool, for allocation of  lots of small objects
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 *
 * (1)通过预分配大段内存以减少内存分配次数
 * (2)节点大小与与预分配的节点数量成反比关系
 * (3)使用位记录某个数组上已删除结点, 如果该数组上所有结点均标记为删除,则删除该条数组
 */
#include "mem_pool.h"
#include <stdlib.h>
#include <unistd.h>    /* usleep */
#include <math.h>    /* log */

#define OBJ_SIZE 1024

struct mem_pool {
	char ** ptr;
	int n;
	int objszie;
};

mem_pool * mp_create(int objsize)
{
	struct mem_pool * mp = (struct mem_pool* )calloc(1, sizeof(struct mem_pool));
	return mp;
}

void * mp_alloc(mem_pool * mp)
{
//	mp->ptr = calloc(1, log(OBJ_SIZE));
}

void mp_free(void * p)
{

}

void mp_destroy(mem_pool * mp)
{
	if(mp && mp->ptr)
		free(mp->ptr);
	free(mp);
}

/* for test */
#if 1
#include <stdio.h>
#include <limits.h>  /* INT_MAX */

struct list_node {
	int id;
	char buf[64];
	short s;
	long l;
	char * p;
};

static int mp_test_main(int argc, char ** argv)
{
	struct mem_pool * mp = mp_create(sizeof(struct list_node));
	if(!mp) return -1;

	int i = 0;
	for(; i < INT_MAX; ++i){
		struct list_node * node = mp_alloct(struct list_node, mp);
		if(!node) break;

		usleep(100000); /* 100ms */

		mp_free(node);
	}

	fprintf(stdout, "%s: alloced count=%d\n", i);

	mp_destroy(mp);
	return 0;
}

#endif
