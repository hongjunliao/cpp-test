/*!
 * This file is PART of landmark_dtu_server
 * @author hongjun.liao <docici@126.com>, @date 2017/05/20
 * dtu module
 */
#if !(defined __GCC__ || defined __CYGWIN__)
#include "lds_dtu_module.h"
#include "lds_inc.h"
#include <windows.h>

typedef BOOL (* DSStartService)(u16t uiListenPort);
typedef BOOL (* DSGetNextData)(ModemDataStruct* pDataStruct,u16t waitseconds);
typedef  BOOL (* DSStopService)(void);
/* handle for dll */
static HMODULE ghdl = 0;

int lds_dtu_start_service(u16t port)
{
	static DSStartService fn = (DSStartService)GetProcAddress(ghdl, "DSStartService");
	return fn && fn(port) == TRUE? 0 : -1;
}

int lds_dtu_stop_service()
{
	static DSStopService fn = (DSStopService)GetProcAddress(ghdl, "DSStopService");
	return fn && fn() == TRUE? 0 : -1;
}

int lds_dtu_get_next_data(ModemDataStruct & d, u16t wait)
{
	static DSGetNextData fn = (DSGetNextData)GetProcAddress(ghdl, "DSGetNextData");
	return fn && fn(&d, wait) == TRUE? 0 : -1;
}

int lds_dtu_module_load(char const * s)
{
	if(!s)
		return -1;
	ghdl= LoadLibrary(s);
	if(!ghdl)
		return -1;
	return 0;
}

#endif /* !(defined __GCC__ || defined __CYGWIN__) */
