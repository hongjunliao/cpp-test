/*
 * API only for win32
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 */
#ifdef WIN32
#include "mywin32_api.h"

int get_nprocs (void)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	count = si.dwNumberOfProcessors;
	return count;
}
#endif

