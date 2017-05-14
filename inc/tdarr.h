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
};

void tdarr_init(tdarr & a, size_t n = 8);
void tdarr_int_add(tdarr & a, int key);

#endif /* DOCICI_TDARRAY_H_ */
