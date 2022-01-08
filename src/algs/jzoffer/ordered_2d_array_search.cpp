/*!
 * This file is PART of docici/cpp-test
 * from <剑指offer>_2.3
 *
 * 二维数组的查找:在一个二维数组中, 每一行都从左到右递增,每一列都从上到下递增;
 * 请为该数据给编写查找算法;
 *
 * sample:
 * 1 2  8  9
 * 2 4  9 12
 * 4 7 10 13
 * 6 8 11 15
 */
#include <stdio.h>

bool ordered_2d_array_search(int * a, size_t m, size_t n, int key)
{
//	fprintf(stdout, "%s: TODO: implement me\n", __FUNCTION__);
//	return false;

	if(!(a && m > 0 && n > 0))
		return false;

	for(size_t i = 0, j = n - 1; i < m && j >= 0;){
		if(key == a[i * m + j])
			return true;
		if(key < a[i * m + j]){
			--j;	/* this col excluded */
			continue;
		}
		++i; /* this row excluded */
	}
	return false;
}

int test_jzoffter_ordered_2d_array_search_main(int argc, char ** argv)
{
	int arr[] { 1, 2, 8, 9, 2, 4, 9, 12, 4, 7, 10, 13, 6, 8, 11, 15 };
	size_t M = 4, N = 4;

	fprintf(stdout, "%s: ordered_2d_array:\n", __FUNCTION__);
	for(size_t i = 0; i < M; ++i){
		for(size_t j = 0; j < N; ++j)
			fprintf(stdout, "%2d ", arr[i * M + j]);
		fprintf(stdout, "\n");
	}
	{
		int key = 7;
		auto r = ordered_2d_array_search(arr, M, N, key);
		fprintf(stdout, "%s: ordered_2d_array_search, key=%d, found = %d\n", __FUNCTION__, key, r);
	}
	{
		int key = 10;
		auto r = ordered_2d_array_search(arr, M, N, key);
		fprintf(stdout, "%s: ordered_2d_array_search, key=%d, found = %d\n", __FUNCTION__, key, r);
	}
	{
		int key = 14;
		auto r = ordered_2d_array_search(arr, M, N, key);
		fprintf(stdout, "%s: ordered_2d_array_search, key=%d, found = %d\n", __FUNCTION__, key, r);
	}
	return 0;
}
