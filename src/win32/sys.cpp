/*
 * API only for win32
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 */
#ifdef WIN32
#include "mywin32_api.h"
#include <windows.h>

int get_nprocs (void)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int count = si.dwNumberOfProcessors;
	return count;
}
#endif /* WIN32 */

