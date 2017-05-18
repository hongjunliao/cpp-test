/*!
 * This file is PART of docici/cpp-test
 * @author hongjun.liao <docici@126.com>
 * 2d-array
 */
#include "tdarr.h"    /* tdarr */
#include <stdio.h>
#include <math.h>     /* log2 */
///////////////////////////////////////////////////////////////////////////////////////////

void tdarr_init(tdarr & a, size_t i, size_t j)
{
	a.i = a.j = 0;
	a.I = i;
	a.J = j;
	a.p = (void **)malloc(a.I * sizeof(void *));
	a.p[0] = 0;
}

int * tdarr_int_new(tdarr & a, int key)
{
	if(!a.p[0]){
		a.p[0] = (int * )malloc(a.J * sizeof(int));
		if(!a.p[0])
			return 0;
	}

	if(a.i == a.I - 1){
		auto I = (size_t)log2(a.I);
		a.I += I < a.I? a.I : I;
		a.p = (void **)realloc(a.p, a.I * sizeof(void *));
		if(!a.p)
			return 0;
	}

	if(a.j == a.J - 1){
		fprintf(stdout, "%s: I=%zu, J=%zu, i=%zu, j=%zu\n", __FUNCTION__, a.I, a.J, a.i, a.j);
		++a.i;
		a.p[a.i] = (int * )malloc(a.J * sizeof(int));
		if(!a.p[a.i])
			return 0;

		a.j = 0;
	}

	auto ret = &((int **)a.p)[a.i][a.j++];
	*ret = key;

	return ret;
}

int test_tdarr_main(int argc, char ** argv)
{
	tdarr arr;
	tdarr_init(arr, 8, 1024 * 1024 * 8 / sizeof(int));
	for(size_t i = 0; tdarr_int_new(arr, i); ++i){
		if((i + 1) / 1000  == 0){
			fprintf(stdout, "\r%s: tdarr_int_new i=%zu", __FUNCTION__, i);
		}
	}
	fprintf(stdout, "\n");

	return 0;
}
