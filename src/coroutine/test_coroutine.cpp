#ifdef CPP_TEST_WITH_COROUTINE

/*
 * test_coroutine.cpp
 *
 *  Created on: Jan 7, 2016
 *      Author: jun
 */
#ifdef __CYGWIN_GCC__
#include "bd_test.h"
#include <ucontext.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
static int test_coroutine_ucontext_basic(int argc, char ** argv);
static int test_coroutine_ucontext_swap(int argc, char ** argv);


static int test_coroutine_ucontext_basic(int argc, char ** argv)
{
	ucontext_t uc;
	getcontext(&uc);
	printf("%s:%s\n", __FUNCTION__, "hello ucontext");
	sleep(1);
	setcontext(&uc);
	return 0;
}
static int do_context_rc = 10;
static void do_context(void)
{
	int r = rand() % 10;
	printf("%s:curr = %d, rand = %d, \n", __FUNCTION__, do_context_rc, r);
	do_context_rc -= r;
	return;
}

static int test_coroutine_ucontext_swap(int argc, char ** argv)
{
	printf("%s: total = %d\n", __FUNCTION__, do_context_rc);
	ucontext_t uc, main_uc;
	getcontext(&uc);
	uc.uc_link = &main_uc;
	makecontext(&uc, do_context, 2, argc, argv);
	/*!此处循环失败*/
	/*for(; do_context_rc > 0; )*/{
		swapcontext(&main_uc, &uc);
		int r = rand() % 5;
		do_context_rc += r;
		printf("%s:curr = %d, rand = %d, \n", __FUNCTION__, do_context_rc, r);
	}
	printf("%s:%s\n", __FUNCTION__, "exit");
	return 0;
}

int test_coroutine_main(int argc, char ** argv)
{
	int r = 0;
	srand(time(NULL));
//	r = test_coroutine_ucontext_basic(argc, argv);
	r = test_coroutine_ucontext_swap(argc, argv);
	return r;
}
#endif /*__CYGWIN_GCC__*/

#endif

