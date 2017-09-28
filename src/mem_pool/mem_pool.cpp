/*!
 * a simple memory pool, for allocation of  lots of small objects
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 *
 * (1)通过预分配大段内存以减少小型对象内存分配次数
 * (2)分配的内存呈二维表格式布局 , 如果当前行已全部使用则分配下一行
 * (3)一行中预分配的节点数量与节点大小大致成倒数关系,以防止对象较大时,预分配的空间浪费
 * (4)每行首部小段空间记录该行上已删除结点, 如果该行上所有结点均标记为删除,则删除该行
 */
#include "mem_pool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>    /* memset */
#include <unistd.h>    /* usleep, sysconf */
#include <math.h>      /* log2 */

#define MAX_OBJ_SIZE 1024
static int verbose  = 0;
static int page_size = 0;

struct mem_pool {
	void ** ptr;             /* start addr of table allocated */
	int x;                   /* current row */
	int y;                   /* current col */
	int X;		             /* total row of allocated*/
	int Y;                   /* total for each col allocated */
	int obj_size;            /* the object size */

	size_t stat_memused;     /* stat, memory used */
};

void mp_set_loglevel(int level)
{
	verbose = level;
}

mem_pool * mp_create(int objsize, int factor)
{
	if(objsize <= 0) return (mem_pool *)0;
	if(!(factor >=1 && factor <= 10)) factor = 5;

	page_size = sysconf(_SC_PAGESIZE);

	struct mem_pool * mp = (struct mem_pool* )calloc(1, sizeof(struct mem_pool));
	if(!mp) return (mem_pool *)0;

	mp->obj_size = objsize;
	mp->Y = (factor * page_size) / (objsize * 10.0) - 2;
	if(mp->Y <= 1)  mp->Y = 2;

	mp->X =  factor * ((log2(objsize) < 8)? 8 : (int)log2(objsize));

	mp->ptr = (void **)calloc(mp->X, sizeof(void *));
	if(!mp->ptr) {
		free(mp);
		return (mem_pool *)0;
	}


	mp->stat_memused = sizeof(struct mem_pool) + mp->X * sizeof(void *);
	if(verbose > 0)
		fprintf(stdout, "%s: X=%d, Y=%d, object_size=%d, memory_used=%zu\n",
				__FUNCTION__, mp->X, mp->Y, mp->obj_size, mp->stat_memused);

	return mp;
}

void * mp_alloc(mem_pool * mp)
{
	if(!mp) return (void *)0;

	if(verbose > 4)
		fprintf(stdout, "%s: X=%d, Y=%d, memory_used=%zu, x=%d, y=%d\n",
				__FUNCTION__, mp->X, mp->Y, mp->stat_memused, mp->x, mp->y);

	if(mp->y == mp->Y - 1){
		++mp->x;
		mp->y = 0;
	}

	if(mp->x == mp->X - 1){
		mp->ptr = (void **)realloc(mp->ptr, mp->X * 2 * sizeof(void *));

		if(!mp->ptr)
			return (void *)0;

		memset((char **)mp->ptr + mp->X, 0, mp->X * sizeof(void *));
		mp->stat_memused += mp->X * sizeof(void *);
		mp->X *= 2;
	}

	int y_hdr = mp->Y * sizeof(char);
	if(!mp->ptr[mp->x]){
		mp->ptr[mp->x] = malloc(y_hdr + mp->Y * mp->obj_size);
		if(!mp->ptr[mp->x])
			return (void *)0;

		memset(mp->ptr[mp->x], -1, y_hdr);
		mp->stat_memused += y_hdr + mp->Y * mp->obj_size;
	}

	void * ret = (char *)(mp->ptr[mp->x]) + y_hdr+ mp->obj_size * mp->y;
	++mp->y;

	return ret;
}

void mp_free(void * p)
{

}

void mp_destroy(mem_pool * mp)
{
	if(!mp) return;
	if(!mp->ptr){
		free(mp);
		return;
	}

	for(int x = 0; x != mp->x; ++x){
		free(mp->ptr[x]);
	}

	free(mp->ptr);
	free(mp);

	return;
}

/* for test */
#if 1
#include <stdio.h>
#include <limits.h>  /* INT_MAX */
#include <time.h>

struct list_node {
	int id;
	char buf[512];
	short s;
	long l;
	char * p;
};

int mp_test_main(int argc, char ** argv)
{
	fprintf(stdout, "%s: usage: %s -n<alloc_count, INT > -m<method, 0: use mempool; 1: use malloc>\n"
			, __FUNCTION__, argv[0]);

	int n = 10000000, method = 0;
	for(int i = 0; i < argc; ++i){
		if(strncmp(argv[i], "-n", 2) == 0)
			n = atoi(argv[i] + 2);
		if(strncmp(argv[i], "-m", 2) == 0)
			method = atoi(argv[i] + 2);
	}
	if(n <= 0) n = 10000000;

	fprintf(stdout, "%s: use method '%s' to alloc objects, count=%d, object_size=%zu, PAGE_SIZE=%d\n"
			, __FUNCTION__, (method == 0? "mempool" : "malloc"), n, sizeof(struct list_node), page_size);

	mp_set_loglevel(1);
	struct mem_pool * mp = mp_create(sizeof(struct list_node), 10);
	if(!mp) return -1;

	int i = 0, failed = 0;
	for(; i < n; ++i){
		struct list_node * node = 0;
		if(method == 0)
			node = mp_alloct(mp, struct list_node);
		else
			node = (struct list_node *)malloc(sizeof(struct list_node));
		if(!node) {
			++failed;
			continue;
		}

		node->id = i;
		sprintf(node->buf, "%d", i);
		node->l = i * 2;
		usleep(1); /* 显示放弃时间片,以模拟制造内存碎片 */

		if(i % 10000 == 0)
			fprintf(stdout, "%s: allocated count=%d, failed=%d, object_content=['%s',%ld,%d]\n"
					, __FUNCTION__,i + 1, failed, node->buf, node->l, node->id);
	}
//	fprintf(stdout, "\n");

	fprintf(stdout, "%s: alloced count=%d\n", __FUNCTION__, i);

	mp_destroy(mp);
	return 0;
}

#endif
