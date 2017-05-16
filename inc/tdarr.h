/*!
 * This file is PART of docici/cpp-test
 * 2d-array
 */
///////////////////////////////////////////////////////////////////////////////////////////
#ifndef DOCICI_TDARRAY_H_
#define DOCICI_TDARRAY_H_
#include <stdlib.h>

struct tdarr {
	void ** p;
	size_t i;
	size_t j;
	size_t I;    /* size for 1d, sample: 8 */
	size_t J;    /* size for 2d, sample: 1024 * 1024 * 8 / sizeof(int) */
};

void tdarr_init(tdarr & a, size_t i, size_t j);
int * tdarr_int_new(tdarr & a, int key);

#endif /* DOCICI_TDARRAY_H_ */
