/*!
 * This file is part of cpp-test
 * @author hongjun.liao <docici@126.com>, @date 2022/2/27
 *
 * from book <Algorithms 4th Edition>
 * 2.X 排序问题
 *
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define algs_sort_t int
typedef struct algs_sort_opt algs_sort_opt;

struct algs_sort_opt {
	//@return true if  arr[m] > arr[n]
	bool (* compare)(void * arr, int m, int n);
	//swap arr[m] and arr[n]
	void (* swap)(void * arr, int m, int n);
	int (* size)();
};
/**
 * @param arr: e.g. 2, 8, 10, 13; 4, 6, 7, 9, 10
 */
static int algs_msort_merge(algs_sort_t * arr, algs_sort_opt opt, int m, int n)
{
	assert(arr && opt.compare && opt.size);

	int * aux = malloc(n * opt.size());
	memcpy(aux, arr, n * opt.size());

	int i = 0, j = m, k;
	for(k = 0; k < n; ++k){
		if(i >= m)                 { arr[k] = aux[j++]; }
		else if(j >= n)            { arr[k] = aux[i++]; }
		else if(opt.compare(aux, i, j)){ arr[k] = aux[j++]; }
		else                      { arr[k] = aux[i++]; }
	}
	free(aux);
	return 0;
}

static int algs_msort_r(algs_sort_t * arr, algs_sort_opt opt, int n)
{
	assert(arr && n >= 0);
	if((n <= 1)) { return 0; }

	int m = n / 2;
	algs_msort_r(arr, opt, m);
	algs_msort_r((int *)arr + m, opt, n - m);
	algs_msort_merge(arr, opt, m, n);
	return 0;
}

static int algs_msort_desc(algs_sort_t * arr, algs_sort_opt opt, int n)
{
	assert(arr && opt.swap);
	int i, m = n / 2;
	for(i = 0; i < m; ++i){
		opt.swap(arr, i, n - 1 - i);
	}
	return 0;
}
/*
 * merge sort
 * */
int algs_msort(algs_sort_t * arr, algs_sort_opt opt, int n, int desc)
{
	if(!(arr && n >= 0 && opt.compare && opt.swap)) { return -1; }
	if((n <= 1)) { return 0; }

	if(algs_msort_r(arr, opt, n) != 0) { return -1; }
	if(desc) { algs_msort_desc(arr, opt, n); }

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
// test begin
static bool int_cmp(void * arr, int m, int n)
{
	assert(arr);
	int *iarr = (int *)arr;
	return iarr[m] > iarr[n];
}

static void int_swap(void * arr, int m, int n)
{
	assert(arr);
	int *iarr = (int *)arr;
	int ex = iarr[m];
	iarr[m] = iarr[n];
	iarr[n] = ex;
}

static int int_size() { return sizeof(int); }

int test_algs4th_sort_main(int argc, char ** argv)
{
	int i, rc = 0;

	algs_sort_opt opt = { int_cmp, int_swap, int_size };
	{
		int vec[] = { 1 };
		rc = algs_msort(vec, opt, 1, 0); assert(rc == 0);
	}
	//2个元素, 升序
	{
		int vec[] = { 1, 2 };
		rc = algs_msort(vec, opt, 2, 0); assert(rc == 0);
		assert(vec[1] == 2 && vec[0] == 1);
	}
	{
		int vec[] = { 2, 1 };
		rc = algs_msort(vec, opt, 2, 0); assert(rc == 0);
		assert(vec[1] == 2 && vec[0] == 1);
	}
	//3个元素, 升序
	{
		int vec[] = { 1, 2, 3 };
		rc = algs_msort(vec, opt, 3, 0); assert(rc == 0);
		assert(vec[0] == 1 && vec[1] == 2 && vec[2] == 3);
	}
	{
		int vec[] = { 2, 3, 1 };
		rc = algs_msort(vec, opt, 3, 0); assert(rc == 0);
		assert(vec[0] == 1 && vec[1] == 2 && vec[2] == 3);
	}
	//4个元素, 升序
	{
		int vec[] = { 3, 4, 2, 1 };
		rc = algs_msort(vec, opt, 4, 0); assert(rc == 0);
		assert(vec[0] == 1 && vec[1] == 2 && vec[2] == 3 && vec[3] == 4);
	}
	{
		int vec[] = {4, 3, 2, 1 };
		rc = algs_msort(vec, opt, 4, 0); assert(rc == 0);
		assert(vec[0] == 1 && vec[1] == 2 && vec[2] == 3 && vec[3] == 4);
	}
	//10个元素, 升序
	{
		int vec[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		rc = algs_msort(vec, opt, 10, 0); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == i); }
	}
	{
		int vec[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
		rc = algs_msort(vec, opt, 10, 0); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == i); }
	}
	{
		int vec[] = { 2, 0, 4, 6, 5, 9, 8, 1, 7, 3 };
		rc = algs_msort(vec, opt, 10, 0); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == i); }
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//2个元素, 降序
	{
		int vec[] = { 1, 2 };
		rc = algs_msort(vec, opt, 2, 1); assert(rc == 0);
		assert(vec[0] == 2 && vec[1] == 1);
	}
	{
		int vec[] = { 2, 1 };
		rc = algs_msort(vec, opt, 2, 1); assert(rc == 0);
		assert(vec[0] == 2 && vec[1] == 1);
	}
	//3个元素, 降序
	{
		int vec[] = { 1, 2, 3 };
		rc = algs_msort(vec, opt, 3, 1); assert(rc == 0);
		assert(vec[0] == 3 && vec[1] == 2 && vec[2] == 1);
	}
	{
		int vec[] = { 2, 3, 1 };
		rc = algs_msort(vec, opt, 3, 1); assert(rc == 0);
		assert(vec[0] == 3 && vec[1] == 2 && vec[2] == 1);
	}
	//4个元素, 降序
	{
		int vec[] = { 3, 4, 2, 1 };
		rc = algs_msort(vec, opt, 4, 1); assert(rc == 0);
		assert(vec[0] == 4 && vec[1] == 3 && vec[2] == 2 && vec[3] == 1);
	}
	//10个元素, 降序
	{
		int vec[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		rc = algs_msort(vec, opt, 10, 1); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == 9 - i); }
	}
	{
		int vec[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
		rc = algs_msort(vec, opt, 10, 1); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == 9 - i); }
	}
	{
		int vec[] = { 2, 0, 4, 6, 5, 9, 8, 1, 7, 3 };
		rc = algs_msort(vec, opt, 10, 1); assert(rc == 0);
		for(i = 0; i < 9; ++i) {  assert(vec[i] == 9 - i); }
	}

	return 0;
}
