/*! This file is PART of cpp-test project
 * @author hongjun.liao <docici@126.com>, @date 2021/6/13
 *
 * 算法竞赛入门经典（第2版） by 刘汝佳 (z-lib.org)
 *
 * */
#include <assert.h> /* assert */
#include <stdio.h>

#define max(a, b) ((a) > (b)? (a) : (b))

#define maxN 5
/* 9.1
 *	d[i][j] = p[i][j] + max(d[i + 1][j], d[i + 1][j + 1]);
 * */
static int solve1_r(int p[][maxN], int i, int j, int n)
{
	return p[i][j] < 0 ? 0 :
			(p[i][j] + max(solve1_r(p, i + 1, j, n), solve1_r(p, i, j + 1, n)));
}

static int solve1(int p[][maxN], int n)
{
	return solve1_r(p, 0, 0, n);
}

static int solve2_r(int p[][maxN], int i, int j, int n, int d[][maxN])
{
	assert(p && i >= 0 && j >= 0 && d && n > 0);
	if(d[i][j] >= 0) { return d[i][j]; }
	return d[i][j] = p[i][j] < 0 ? 0 :
			(p[i][j] + max(solve1_r(p, i + 1, j, n), solve1_r(p, i, j + 1, n)));
}

static int solve2(int p[][maxN], int d[][maxN], int n)
{
	for(int i = 0; i < n; ++i) {
		for(int j = n - 1 - i; j >= 0; --j){
			d[i][j] = -1;
		}
	}
	return solve2_r(p, 0, 0, n, d);
}

static int solve3(int p[][maxN], int d[][maxN], int n)
{
	for(int k = n - 1; k >= 0; --k){
		for(int i = k, j =  0; i >= 0 && j <= k; --i, ++j){
			d[i][j] = (i + j == n - 1)? p[i][j] :
				(p[i][j] + max(d[i + 1][j], d[i][j + 1]));
		}
	}
	return d[0][0];
}

int test_max_subarray_main(int argc, char ** argv)
{
	int rec[][maxN] = {
		  {1, 2, 1, 20, -1}
		, {3, 10, 2, -1, -1}
		, {4, 3, -1, -1, -1}
		, {4, -1, -1, -1, -1}
		, {-1}
	};
	int r;
	r = solve1(rec, 4);
	assert(r == 24);

	{
		int D[maxN][maxN];
		r = solve2(rec, D, 4);
		assert(r == 24);
	}
	{
		int D[maxN][maxN];
		r = solve3(rec, D, 4);
		assert(r == 24);
	}

	return 0;
}
