/*!
 * This file is part of cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2022/1/7
 *
 * from book <Algorithms 4th Edition>
 * 1.5 案例:动态连通分量问题
 * 输入是一系列整数对, 每对整数p,p(可称为"触点")表示p,q是"连通"的,连通性具有传递性,一个该集合称为一个"连通分量",设计算法:
 * 查询特定整数对(两个触点)是否连通,添加到现有连通分量
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////

#include "c-vector/cvector.h"
#define max(a,b) ((a) > (b) ? a : b)
/*
 * Weighted Quick Union, 带权重的
 * */
typedef struct wqu_s wqu_s;
struct wqu_s {
	/* 下标和值均为触点ID标识 */
	int * ids;
	/* 当前最大触点ID, 用于动态调整ids */
	int maxid;

	int * sz;
	int count;
};

/* 动态调整下触点触点容量,并初始化"新"触点 */
static int wqu_grow(wqu_s * wqu, int maxid)
{
	if(!(wqu && maxid >= 0)) { return -1; }
	assert((wqu->ids));

	int i;
	if(cvector_capacity(wqu->ids) < maxid){
		cvector_grow(wqu->ids, maxid);
		/* 触点初始时未与其它任何触点连通 */
		for(i = wqu->maxid + 1; i < maxid; ++i){
			wqu->ids[i] = i;
		}
		wqu->maxid = maxid -1;
	}
	return 0;
}

/*
 * 查找触点所在的连通分量 */
static int wqu_find(wqu_s * wqu, int p)
{
	assert(wqu);
	if(!(p >= 0 && p <= wqu->maxid)) { return p; }
	for(;wqu->ids[p] != p;) {
		p = wqu->ids[p];
	}
	return p;
}

int wqu_init(wqu_s * wqu)
{
	if(!wqu) { return -1; }
	memset(wqu, 0, sizeof(wqu_s));
	cvector_init(wqu->ids, 1);
	cvector_init(wqu->sz, 1);
	wqu->maxid = 0;

	/* 初始条件成立:触点0初始时未与其它任何触点连通 */
	wqu->ids[0] = 0;

	return 0;
}

int wqu_union(wqu_s * wqu, int p, int q)
{
	if(!wqu) { return -1; }
	assert((wqu->ids));

	int maxid = max(p, q) + 1;
	if(wqu_grow(wqu, maxid) != 0) { return -2; }

	int r = wqu_find(wqu, p), s = wqu_find(wqu, q);
	if(r == s) { return 0; }  /* 已经是连通的,返回 */

	wqu->ids[q] = p;

	return 0;
}

int wqu_is_connected(wqu_s * wqu, int p, int q)
{
	return wqu_find(wqu, p) == wqu_find(wqu, q);
}

int wqu_count(wqu_s * wqu)
{
	return 0;
}

void wqu_uninit(wqu_s * wqu)
{
	if(!wqu) { return; }
	cvector_free(wqu->ids);
	cvector_free(wqu->sz);

	return;
}

int test_algorithms4th_1_5_main(int argc, char ** argv)
{
	int rc;
	{
		wqu_s uobj, * pu =&uobj;
		rc = wqu_init(pu); assert(rc == 0);
		assert(!wqu_is_connected(pu, 1, 0));
		rc = wqu_union(pu, 0, 1); assert(rc == 0);
		rc = wqu_union(pu, 1, 0); assert(rc == 0);
		assert(wqu_is_connected(pu, 1, 0));
		wqu_uninit(pu);
	}
	{
		wqu_s uobj, * pu =&uobj;
		rc = wqu_init(pu); assert(rc == 0);
		rc = wqu_union(pu, 0, 1); assert(rc == 0);
		rc = wqu_union(pu, 0, 1); assert(rc == 0);
		rc = wqu_union(pu, 0, 2); assert(rc == 0);
		assert(wqu_is_connected(pu, 1, 2));
		assert(!wqu_is_connected(pu, 1, 3));
		wqu_uninit(pu);
	}

	return 0;
}
