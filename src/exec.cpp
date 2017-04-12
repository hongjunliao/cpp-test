/*!
 * This File is PART of file-sync project/rsync-wrapper-module
 * @author hongjun.liao <docici@126.com>
 * @date 2016/11
 *
 * ONLY on Win32!
 * use CreateProcess and CreatePipe to create a process and read it's output
 */

/*!
 * This file is PART of speedtop-uploader/cygwin
 * @author hongjun.liao <docici@126.com>
 * @date 2016/12
 * @notes:
 * 1.original from file-sync project/rsync-wrapper-module
 */
#if (defined __CYGWIN__ || defined _WIN32)

#include "process_util.h"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

static char const * get_last_err_msg();

int execute_with_wait_pipe(char const * dir, char * cmd, char * buff, int & length,
		void (*stdout_cb)(char const * data, int length, void * arg),
		void * arg)
{
	fprintf(stdout, "%s: cmd=%s\n", __FUNCTION__, cmd);
	if(!buff || length <= 0)
		return -1;

	HANDLE hReadPipe;
	HANDLE hWritePipe;
	SECURITY_ATTRIBUTES sa = { 0};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	DWORD nSize = 0;
	BOOL r = ::CreatePipe(&hReadPipe, &hWritePipe, &sa, nSize);
	if(!r){
		fprintf(stderr, "%s: CreatePipe failed\n", __FUNCTION__);
		return 1;
	}

	STARTUPINFO si  = {0};
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = hWritePipe; //
	si.hStdOutput = hWritePipe; //
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi;
	r = CreateProcess(NULL, (LPTSTR)cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, dir, &si, &pi);
	CloseHandle(hWritePipe);	//@IMPORTANT! or ReadFile() will blocked
	if (!r) {
		fprintf(stderr, "%s: CreateProcess failed\n", __FUNCTION__);
		return 1;
	}

	int proc_exit = 0;
	for(;;){
		DWORD dw = WaitForSingleObject(hReadPipe, 80);
		if(dw == WAIT_OBJECT_0){
			proc_exit = (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0);
			if(proc_exit){
				//TODO: GetExitCodeProcess?
				fprintf(stderr, "\n%s: process finished\n", __FUNCTION__);
			}
			DWORD bytes_read = 0;
			BOOL r = ReadFile(hReadPipe, buff, length, &bytes_read, NULL);
			if(!r){
				char const * err_msg = get_last_err_msg();
				for(int left = 3 * 1000; proc_exit != 1 && left > 0; left -= 99){
					DWORD result = WaitForSingleObject(pi.hProcess, 100);
					proc_exit = (result == WAIT_OBJECT_0? 1 : (result == WAIT_TIMEOUT? 0 : 2));
					if(proc_exit == 2)	//error
						break;
				}
				fprintf(stderr, "%s: ReadFile failed, error=%s, proc exit = %d\n", __FUNCTION__, err_msg, proc_exit);
				break;
			}
			if(bytes_read == 0){
				continue;
			}
			length = bytes_read;
			buff[bytes_read] = 0;
			if(stdout_cb)
				stdout_cb(buff, bytes_read, arg);
		}
		else if(dw == WAIT_TIMEOUT){
			fprintf(stderr, "%s: WaitForSingleObject timeout\n", __FUNCTION__);
			continue;
		}
		else if(dw == WAIT_FAILED){
			fprintf(stderr, "%s: WaitForSingleObject failed!!\n", __FUNCTION__);
			break;
		}
		else{
			fprintf(stderr, "%s: WaitForMultipleObjects internal error\n", __FUNCTION__);
			break;
		}
	}
	fprintf(stdout, "%s: done, cmd='%s', return = %d\n", __FUNCTION__, cmd, 0);
	return 0;
}

char const * get_last_err_msg()
{
	static char errstr[512] = _T("");
	// Get the error code
	DWORD dwError = GetLastError();
	HLOCAL hlocal = NULL;   // Buffer that gets the error message string

	// Use the default system locale since we look for Windows messages.
	// Note: this MAKELANGID combination has 0 as value
	DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

	// Get the error code's textual description
	BOOL fOk = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, dwError, systemLocale,
		(PTSTR) &hlocal, 0, NULL);

	if (!fOk) {
		// Is it a network-related error?
		HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL,
			DONT_RESOLVE_DLL_REFERENCES);

		if (hDll != NULL) {
			fOk = FormatMessage(
				FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				hDll, dwError, systemLocale,
				(PTSTR) &hlocal, 0, NULL);
			FreeLibrary(hDll);
		}
	}

	if (fOk && (hlocal != NULL)) {
		strncpy(errstr, (char const *) LocalLock(hlocal), 512 - 1);
		LocalFree(hlocal);
	}
	return errstr;
}

bool is_os_version_major_less_equal(int ver)
{

	OSVERSIONINFOEX osvi = {0};
	osvi.dwMajorVersion= ver;
	DWORDLONG dwlConditionMask = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_LESS_EQUAL);
	auto result = (::VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask) != FALSE);
	return result;
	return false;
}

#endif	/* defined __CYGWIN__ || defined WIN32 */
