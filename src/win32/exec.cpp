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
#include "string_util.h" /* str_dump */
#if  (defined __CYGWIN__ || defined WIN32)

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
	BOOL result = (::VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask) != FALSE);
	return result;
	return false;
}

#endif	/* defined __CYGWIN__ || defined WIN32 */

#define is_intercmdctx_ok(ctx) (ctx.dir && ctx.dir[0] != '\0' && ctx.cmd && ctx.cmd[0] != '\0' && ctx.direct && ctx.direct[0] != '\0')
#define is_intercmdctx_ready(ctx) (ctx.hproc && ctx.rh && ctx.wh)

/* @see sample code from https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx */
static int exec_intercmd(intercmdctx & ctx)
{
	char const * dir = ctx.dir, * cmd = ctx.cmd;
	fprintf(stdout, "%s: dir='%s', cmd='%s'\n", __FUNCTION__, dir, cmd);
	if(!(dir && dir[0] != '\0' && cmd && cmd[0] != '\0'))
		return -1;

	HANDLE fd1[2];
	HANDLE fd2[2];
	SECURITY_ATTRIBUTES sa = { 0};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor  = 0;

	BOOL r = ::CreatePipe(&fd1[0], &fd1[1], &sa, 0);
	BOOL r2 = ::CreatePipe(&fd2[0], &fd2[1], &sa, 0);
	if(!(r == TRUE && r2 == TRUE &&
			SetHandleInformation(fd1[0], HANDLE_FLAG_INHERIT, 0) &&
			SetHandleInformation(fd2[1], HANDLE_FLAG_INHERIT, 0))){
		fprintf(stderr, "%s: CreatePipe failed\n", __FUNCTION__);
		return -1;
	}

	STARTUPINFO si  = {0};
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = fd1[1];
	si.hStdOutput = fd1[1];
	si.hStdInput = fd2[0];
	si.dwFlags |= STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi;
	r = CreateProcess(NULL, (LPTSTR)cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, dir, &si, &pi);
	if (!r) {
		fprintf(stderr, "%s: CreateProcess failed\n", __FUNCTION__);
		return -1;
	}

	ctx.hproc = pi.hProcess;
	ctx.rh = fd1[0];
	ctx.wh = fd2[1];
	return 0;
}

int send_intercmd(intercmdctx & ctx, char * buf, size_t length,
		int (*stdout_cb)(char const * data, size_t length, void * arg), void * arg)
{
	if(!is_intercmdctx_ok(ctx))
		return -1;
	if(!is_intercmdctx_ready(ctx)){
		int r = exec_intercmd(ctx);
		if(r != 0){
			fprintf(stderr, "%s: exec_intercmd failed\n", __FUNCTION__);
			return -1;
		}
	}

	DWORD ec;
	GetExitCodeProcess(ctx.hproc, &ec);
	if(ec != STILL_ACTIVE){
		ctx.hproc = 0;
		return -1;
	}

//	str_dump(stdout, ctx.direct, strlen(ctx.direct), __FUNCTION__, "_______\n");

	DWORD written = 0;
	BOOL r = WriteFile(ctx.wh, ctx.direct, strlen(ctx.direct), &written, NULL);
	if(r != TRUE){
		fprintf(stdout, "%s: failed, written=%ld\n", __FUNCTION__, written);
		return -1;
	}
	if(ctx.exit)
		return 0;
	if(!(buf && length > 0))
		return 0;

	for(;;){
		DWORD dw = WaitForSingleObject(ctx.wh, 80);
		if(dw == WAIT_OBJECT_0){
			DWORD r = (WaitForSingleObject(ctx.hproc, 0) == WAIT_OBJECT_0);
			if(r){
				//TODO: GetExitCodeProcess?
				fprintf(stdout, "%s: process finished\n", __FUNCTION__);
				ctx.hproc = 0;
				CloseHandle(ctx.wh);
			}

			DWORD bytes_read = 0;
			r = ReadFile(ctx.rh, buf, length, &bytes_read, NULL);
			if(r != TRUE)
				return -1;
			if(bytes_read == 0)
				continue;

			buf[bytes_read] = '\0';
			if(stdout_cb && stdout_cb(buf, bytes_read, arg) == 2)
				return 0;
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
			return -1;
		}
	}

	fprintf(stdout, "%s: done, written=%ld\n", __FUNCTION__, written);
	return 0;
}

#ifdef DEBUG

static int sample_intercmd_stdout(char const * data, size_t len, void * arg)
{
//	fprintf(stdout, "%s", data);
	str_dump(stdout, data, len, 0, "\n");
	if(data && len > 2){
		if(data[len - 3] == ')' && data[len - 1] == '\n'){
			fprintf(stdout, "%s: _________________________\n", __FUNCTION__);
			return 2;
		}
	}
	return 0;
}

int test_exec_main(int argc, char ** argv)
{
	intercmdctx ctx = { 0 };
	ctx.dir = "D:/ws/release/";
	ctx.cmd = "sqlcmd";
	char buf[1024 * 10];
	/* sql select */
	ctx.direct = "select top 1 A,PH from waterparameter001\ngo\n";
	int r = send_intercmd(ctx, buf, sizeof(buf), sample_intercmd_stdout, 0);
	if(r != 0){
		fprintf(stderr, "%s: send_intercmd_direct failed\n", __FUNCTION__);
		return -1;
	}

	/* sql insert */
	ctx.direct = "exec sp_insert_wpp 1,'2012-04-06 15:04:03','0','A',24.5,76.1,517.8,7.48,7.35,4.01,0.0, 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,'9','1'";
	r = send_intercmd(ctx, buf, sizeof(buf), sample_intercmd_stdout, 0);
	if(r != 0){
		fprintf(stderr, "%s: send_intercmd_direct failed\n", __FUNCTION__);
		return -1;
	}

	/* send exit direct */
	ctx.direct = "exit\n";
	ctx.exit = true;
	r = send_intercmd(ctx, buf, sizeof(buf), sample_intercmd_stdout, 0);
	if(r != 0){
		fprintf(stderr, "%s: send_intercmd_direct failed\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
#else
int test_exec_main(int argc, char ** argv)
{
	fprintf(stderr, "%s: define DEBUG to open this test\n", __FUNCTION__);
	return -1;
}
#endif /* DEBUG */
