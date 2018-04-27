/*!
 * This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2017/9/7
 *
 * a simple memory pool, for allocation of  fixed-size objects
 *
 * @history:
 * 2018/4/28 change to c
 *
 * (1)通过预分配大段内存以减少小型对象内存分配次数
 * (2)分配的内存呈二维表格式布局 , 如果当前行已全部使用则分配下一行
 * (3)一行中预分配的节点数量与节点大小大致成倒数关系,以防止对象较大时,预分配的空间浪费
 * (4)每行首部小段空间记录该行上已删除结点, 如果该行上所有结点均标记为删除,则删除该行
 */
#include "mem_pool.h"
#include "cp_log.h"    /* cp_logf,... */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>    /* memset */
#include <unistd.h>    /* usleep, sysconf */
#include <math.h>      /* log2 */
#include <assert.h>    /* assert */

#ifndef NDEBUG
static char const * mp_sdump_hdr(struct mem_pool * mp, int c
		, char * out, int len
		, char const * beg, char const * end);
#endif /* NDEBUG */

struct mem_pool {
	char ** ptr;             /* start addr of table allocated */
	int x;                   /* current row */
	int y;                   /* current col */
	int NX;		             /* total row of allocated*/
	int NY;                  /* total for each col allocated */
	int NOBJ;                /* the object size */
	size_t memused;          /* stat, memory used */
};

struct mem_pool * mp_create(int objsize, int y)
{
	if(objsize <= 0)
		return (struct mem_pool *)0;

	if(y < 1)
		y = 1;

	struct mem_pool * mp = (struct mem_pool* )calloc(1, sizeof(struct mem_pool));
	assert(mp);

	mp->NOBJ = objsize;
	mp->NY = y;
	mp->NX = 10;
	mp->ptr = (char **)calloc(mp->NX, sizeof(char *));
	assert(mp->ptr);

	mp->memused = sizeof(struct mem_pool) + mp->NX * sizeof(void *);
#ifndef NDEBUG
		fprintf(stdout, "%s: X=%d, Y=%d, object_size=%d, factor=%d, memory_used=%zu\n",
				__FUNCTION__, mp->NX, mp->NY, mp->NOBJ, y, mp->memused);
#endif /* NDEBUG */
	return mp;
}

void * mp_alloc(struct mem_pool * mp)
{
	if(!mp)
		return (void *)0;

	if(mp->x == mp->NX - 1){
#ifndef NDEBUG
	fprintf(stdout, "%s: realloc X, X=%d\n", __FUNCTION__, mp->NX);
#endif /* NDEBUG */
		mp->ptr = (char **)realloc(mp->ptr, mp->NX * 2 * sizeof(char *));
		if(!mp->ptr)
			return 0;

		memset((char **)mp->ptr + mp->NX, 0, mp->NX * sizeof(void *));
		mp->memused += mp->NX * sizeof(void *);
		mp->NX *= 2;
	}

	int HDRLEN = mp->NY * sizeof(char **);
	char * row = mp->ptr[mp->x];
	if(!row){
		row = (char *)malloc(HDRLEN + mp->NY * mp->NOBJ);
		if(!row)
			return 0;

		memset(row, 0, HDRLEN);
		mp->ptr[mp->x] = row;
		mp->memused += HDRLEN + mp->NY * mp->NOBJ;
	}

	char * addr = row + HDRLEN + mp->NOBJ * mp->y;
	char ** hdr = (char ** )row;
	hdr[mp->y] = addr;

#ifndef NDEBUG
//	char buf[512];
//	buf[0] = '\0';
//	fprintf(stdout, "%s: X=%d, Y=%d, x=%d, y=%d, hdr=%s/%d"
//			", addr=%p, memory_used=%zu\n",
//			__FUNCTION__, mp->NX, mp->NY, mp->x, mp->y
//			, mp_sdump_hdr(mp, mp->NY, buf, sizeof(buf), "[", "]"), HDRLEN
//			, addr, mp->memused);
#endif /* NDEBUG */

	++mp->y;
	if(mp->y == mp->NY){
		++mp->x;
		mp->y = 0;
	}

	return addr;
}

void mp_free(struct mem_pool * mp, void * p)
{
	int x, h;
	for(x = 0; mp->ptr + x; ++x){
		char ** row = (char **)mp->ptr + x;
		for(h = 0; row + h; ++h){
			if(row + h == p){
				row[h] = 0;
				return;
			}
		}
	}
}

void mp_destroy(struct mem_pool * mp)
{
	if(!mp)
		return;
#ifndef NDEBUG
	char buf[512];
	buf[0] = '\0';
	fprintf(stdout, "%s: X=%d, Y=%d, x=%d, y=%d, hdr=%s"
			", memory_used=%zu\n",
			__FUNCTION__, mp->NX, mp->NY, mp->x, mp->y
			, mp_sdump_hdr(mp, mp->NY, buf, sizeof(buf), "[", "]")
			, mp->memused);
#endif /* NDEBUG */
	int x;
	for(x = 0; x != mp->x; ++x)
		free(mp->ptr[x]);
	free(mp->ptr);
	free(mp);

	return;
}

#ifndef NDEBUG
static char const * mp_sdump_hdr(struct mem_pool * mp, int c
		, char * out, int len
		, char const * beg, char const * end)
{
	if(!(mp && c > 0 && out && len > 0))
		return "";

	out[0] = '\0';
	if(c > mp->NY)
		c = mp->NY;

	char ** row = ( char ** )mp->ptr[mp->x];
	if(!row){
		snprintf(out, len, "%s%s", beg, end);
		return out;
	}

	int elen = (end? strlen(end) : 0);
	int i;
	int n = 0;

	n += snprintf(out + n, len - n, "%s", beg);
	for(i = 0; i < c; ++i){
		if(len - (n + 10 + elen) <= 0){
			n += snprintf(out + n, len - n, "%s", "...");
			break;
		}
		n += snprintf(out + n, len - n,  "%p, ", row[i]);
	}
	n += snprintf(out + n, len - n, "%s", end);

	return out;
}
#endif /* NDEBUG */

/////////////////////////////////////////////////////////////////////////////////////
/* for test */
#ifndef NDEBUG
#include <stdio.h>
#include <limits.h>  /* INT_MAX */
#include <time.h>

struct list_node {
	int i;
	char buf[512];
	short s;
	long l;
	char * p;
};

struct client_node {
	int i;
	char buf[1024 * 16];
	short s;
	long l;
	char * p;
};

int mp_test1_main(int argc, char ** argv)
{
	int n = 10000000, method = 0;
	for(int i = 0; i < argc; ++i){
		if(strncmp(argv[i], "-n", 2) == 0)
			n = atoi(argv[i] + 2);
		if(strncmp(argv[i], "-m", 2) == 0)
			method = atoi(argv[i] + 2);
	}
	if(n <= 0) n = 10000000;

	fprintf(stdout, "%s: use method '%s' to alloc objects, count=%d, object_size=%zu, PAGE_SIZE=%ld\n"
			, __FUNCTION__, (method == 0? "mempool" : "malloc"), n, sizeof(struct list_node), sysconf(_SC_PAGESIZE));

	struct mem_pool * mp = mp_create(sizeof(struct list_node), 50);
	if(!mp) return -1;

	time_t startt = time(0);
	int i = 0, failed = 0;
	for(; i < n; ++i){
		struct list_node * node = 0;
		if(method == 0)
			node = (struct list_node *)mp_alloc(mp);
		else
			node = (struct list_node *)malloc(sizeof(struct list_node));
		if(!node) {
			++failed;
			continue;
		}

		node->i = i;
		sprintf(node->buf, "%d", i);
		node->l = i * 2;
		usleep(1); /* 显示放弃时间片,以模拟制造内存碎片 */

		time_t now = time(0);
		if(now - startt > 0){
			fprintf(stdout, "%s: allocated count=%d, failed=%d, object_content=['%s',%ld,%d]\n"
					, __FUNCTION__,i + 1, failed, node->buf, node->l, node->i);
			startt = now;
		}
	}

	fprintf(stdout, "%s: alloc done! count=%d\n", __FUNCTION__, i);

	mp_destroy(mp);
	return 0;
}

int mp_test2_main(int argc, char ** argv)
{
	int n = 10000000, method = 0;
	for(int i = 0; i < argc; ++i){
		if(strncmp(argv[i], "-n", 2) == 0)
			n = atoi(argv[i] + 2);
		if(strncmp(argv[i], "-m", 2) == 0)
			method = atoi(argv[i] + 2);
	}
	if(n <= 0) n = 10000000;

	fprintf(stdout, "%s: use method '%s' to alloc objects, count=%d, object_size=%zu, PAGE_SIZE=%ld\n"
			, __FUNCTION__, (method == 0? "mempool" : "malloc"), n, sizeof(struct client_node), sysconf(_SC_PAGESIZE));

	struct mem_pool * mp = mp_create(sizeof(struct client_node), 128);
	if(!mp) return -1;

	time_t startt = time(0);
	int i = 0, failed = 0;
	for(; i < n; ++i){
		struct client_node * node = 0;
		if(method == 0)
			node = (struct client_node *)mp_alloc(mp);
		else
			node = (struct client_node *)malloc(sizeof(struct client_node));
		if(!node) {
			++failed;
			continue;
		}

		node->i = i;
		sprintf(node->buf, "%d", i);
		usleep(1); /* 显示放弃时间片,以模拟制造内存碎片 */

		time_t now = time(0);
		if(now - startt > 0){
			fprintf(stdout, "%s: allocated count=%d, failed=%d, object_content=['%s',%ld,%d]\n"
					, __FUNCTION__,i + 1, failed, node->buf, node->l, node->i);
			startt = now;
		}
	}

	fprintf(stdout, "%s: alloc done! count=%d\n", __FUNCTION__, i);

	mp_destroy(mp);
	return 0;
}

int mp_test_main(int argc, char ** argv)
{
	if(mp_test1_main(argc, argv) != 0) return -1;
	if(mp_test2_main(argc, argv) != 0) return -2;
	return 0;
}
char const * help_mp_test_main()
{
	return "-n<alloc_count, INT > -m<method, 0: use mempool; 1: use malloc>\n";
}
#endif /* NDEBUG */
