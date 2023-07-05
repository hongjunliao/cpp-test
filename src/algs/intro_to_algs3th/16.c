/*!
 * test greedy/贪心算法 from <Introduction_to_Algorithms 3th Edition>_chapter_16
 * @author hongjun.liao <docici@126.com>, @date 2022/6/25
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hp/hp_log.h"
#include <stdio.h>
#include <stdarg.h>     /* va_list, ... */
#include <time.h>       /* time.h */
#include <locale.h>
#ifndef _MSC_VER
#include <unistd.h>     /* getpid, isatty */
#include <sys/time.h>   /* gettimeofday */
#include "sds/sds.h"        /* sds */
#else
#include <windows.h>
#include <io.h>         /* isatty */
#include <sysinfoapi.h> /* GetTickCount */
#include <processthreadsapi.h> /* GetCurrentThreadId */
#include "hp/sdsinc.h"		/* sds */
#endif /* _MSC_VER */

///////////////////////////////////////////////////////////////////////////////////////////

/**
 * 16.1活动选择问题：
 * 找出最大兼容活动子集
 */

typedef struct itoa_activity itoa_activity;
struct itoa_activity{
	int s, f; /* 开始,结束时间 */
	int chosen;
};

static itoa_activity * longest_activity(int s, int f, itoa_activity * as, int nas)
{
	assert(as);
	int i;
	if(s == f) return 0;
	if(nas == 0) return 0;

	itoa_activity * a = 0;
	int maxa = 0;
	for(i = 0; i < nas; ++i){
		if(!as[i].chosen){
			if(as[i].s >= s && as[i].f <= f){
				int tmaxa = as[i].f - as[i].s;
				if(tmaxa > maxa){
					maxa = tmaxa;
					a = as + i;
				}
			}
		}
	}

	if(a)
		a->chosen = 1;

	return a;
}

void itoa_activity_dump(FILE * f, itoa_activity * a, char c)
{
	fprintf(f, "{%d,%d}%c", a->s, a->f, c);
}

int max_comp_activity(int s, int f, itoa_activity * as, int nas
		, itoa_activity *comp_as, int * ias)
{
	if(s == f) return 0;
	if(nas == 0) return 0;

	itoa_activity * a = longest_activity(s, f, as, nas);
	if(a){
		comp_as[*ias] = *a; ++*ias;
		if(a->s == s && a->f == f)
			return 0;
		else if(a->s == s && a->f < f)
			return max_comp_activity(a->f, f, as, nas, comp_as, ias);
		else if(a->s > s && a->f == f)
			return max_comp_activity(s, a->s, as, nas, comp_as, ias);
		else{
			max_comp_activity(s, a->s, as, nas, comp_as, ias);
			max_comp_activity(a->f, f, as, nas, comp_as, ias);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////

int test_greedy_main(int argc, char ** argv)
{
	int ret = 0;
	int i;
	hp_log(stdout, "%s:\n", __FUNCTION__);

	int s = 1, f = 11;
	itoa_activity as[] = {
			{1, 4}, {3, 5}, {0, 6}, {5, 7}, {3, 9}, {5, 9}, {6, 10}, {8, 11}
			, {8, 12}, {2, 14}, {12, 16}
	}, comp_as[sizeof(as)];
	int ncomp_as = 0;
	ret = max_comp_activity(s, f, as, sizeof(as), comp_as, &ncomp_as);

	hp_log(stdout, "%s: ncomp_as = %d, ", __FUNCTION__, ncomp_as);
	for(i = 0; i < ncomp_as; ++i){
		itoa_activity_dump(stdout, comp_as + i, (i == ncomp_as - 1? '\n' : ','));
	}

	exit(0);

	return ret;
}
