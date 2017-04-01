#ifdef __CYGWIN__
#include "plcdn_cpp_test.h"	/* test_win32_pipe_main */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/*with BUGS*/
int execute_with_pipe(char * cmd, void (*stdout_cb)(void * data, int length));

int execute_with_wait_pipe(char * cmd, void (*stdout_cb)(void * data, int length));

char const * get_last_err_msg();

static void get_stdout(void * data, int len)
{
	fprintf(stdout, "%s", (char * )data);
	fflush(stdout);
}


int execute_with_pipe(char * cmd, void (*stdout_cb)(void * data, int length))
{
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
	si.hStdError = hWritePipe; //设定其标准错误输出为hWritePipe
	si.hStdOutput = hWritePipe; //设定其标准输出为hWritePipe
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi;
	r = CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	if (!r) {
		fprintf(stderr, "%s: CreateProcess failed\n", __FUNCTION__);
		return 1;
	}
	for(bool keep = true; keep;){
		keep = false;
		DWORD dw = WaitForSingleObject(pi.hProcess, 20);
		switch(dw){
		case WAIT_FAILED:{
			fprintf(stderr, "%s: WaitForSingleObject failed\n", __FUNCTION__);
			return 1;
		}
		case WAIT_TIMEOUT:
		case WAIT_OBJECT_0:{
			char readbuff[1024 * 5] = "";
			DWORD bytesRead = 0;
			BOOL r = ReadFile(hReadPipe, readbuff, sizeof(readbuff), &bytesRead, NULL);
			if(!r){
				fprintf(stderr, "%s: ReadFile failed\n", __FUNCTION__);
				return 1;
			}
			readbuff[bytesRead] = 0;

			if(bytesRead != 0){
				stdout_cb(readbuff, bytesRead);
			}
			if(dw == WAIT_OBJECT_0){
				CloseHandle(hWritePipe);
				fprintf(stderr, "%s: process finished\n", __FUNCTION__);
			}
			else
				keep = true;
			break;
		}
		default:
			fprintf(stderr, "%s: WaitForSingleObject internal error\n", __FUNCTION__);
			break;
		}
	}
	return 0;
}

int test_win32_pipe_main(int argc, char ** argv)
{
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
	char buff[] = "hello, pipe";
	DWORD bytesWrite = 0;
	r = WriteFile(hWritePipe, buff, sizeof(buff), &bytesWrite, NULL);
	if(!r){
		fprintf(stderr, "%s: WriteFile failed\n", __FUNCTION__);
		return 1;
	}
	fprintf(stdout, "%s: write to pipe: %s, size = %d\n", __FUNCTION__, buff, bytesWrite);
//	CloseHandle(hWritePipe);
	char readbuff[1024 * 5] = "";
	DWORD bytesRead = 0;
	r = ReadFile(hReadPipe, readbuff, sizeof(readbuff), &bytesRead, NULL);
	if(!r){
		fprintf(stderr, "%s: ReadFile failed\n", __FUNCTION__);
		return 1;
	}
	fprintf(stdout, "%s: read from pipe: %s, size = %d\n", __FUNCTION__, readbuff, bytesRead);
	return 0;
}

int test_subprocess_with_pipe_main(int argc, char ** argv)
{
	char defcmd[512] = "git --help";
	char * cmd = argc > 1? argv[1] : defcmd;
	fprintf(stdout, "%s: cmd = %s\n", __FUNCTION__, cmd);
	execute_with_pipe(cmd, get_stdout);
	return 0;
}

int execute_with_wait_pipe(char * cmd, void (*stdout_cb)(void * data, int length))
{
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
	si.hStdError = hWritePipe; //设定其标准错误输出为hWritePipe
	si.hStdOutput = hWritePipe; //设定其标准输出为hWritePipe
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES;

	PROCESS_INFORMATION pi;
	r = CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
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
				fprintf(stderr, "\n%s: process finished\n", __FUNCTION__);
			}
			char readbuff[1024 * 5] = "";
			DWORD bytesRead = 0;
			BOOL r = ReadFile(hReadPipe, readbuff, sizeof(readbuff), &bytesRead, NULL);
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
			if(bytesRead == 0){
				continue;
			}
			readbuff[bytesRead] = 0;
			stdout_cb(readbuff, bytesRead);
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
	return 0;
}
int test_subprocess_wait_pipe_handle_main(int argc, char ** argv)
{
	char defcmd[512] = "git --help";
	char * cmd = argc > 1? argv[1] : defcmd;
	fprintf(stdout, "%s: cmd = %s\n", __FUNCTION__, cmd);

	return execute_with_wait_pipe(cmd, get_stdout);

}

char const * get_last_err_msg()
{
	static char errstr[512] = "";
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
       strncpy(errstr, (PCTSTR) LocalLock(hlocal), 512 - 1);
       LocalFree(hlocal);
    }
    return errstr;
}

int test_process_ioredir_with_file()
{
	STARTUPINFO   startupinfo = {  0 };
	startupinfo.cb=sizeof(STARTUPINFO);
//	GetStartupInfo(&startupinfo);
	startupinfo.dwFlags=STARTF_USESTDHANDLES;
	startupinfo.wShowWindow=SW_HIDE;

	SECURITY_ATTRIBUTES psa={sizeof(psa),NULL,TRUE};;
	psa.bInheritHandle=TRUE;

	char const * outfile = "rsync.log";

	HANDLE houtputfile = CreateFile(outfile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &psa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(houtputfile == INVALID_HANDLE_VALUE){
		fprintf(stderr, "create file error\n");
		return GetLastError();
	}
	startupinfo.hStdOutput = houtputfile;
	startupinfo.hStdError = houtputfile;


	PROCESS_INFORMATION   pinfo;
	char szcmdline[512] = "ls";
	if(!CreateProcess(NULL, (LPSTR)szcmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &startupinfo, &pinfo))
	{
		fprintf(stderr, "CreateProcess failed\n");
		return GetLastError();
	}
	WaitForSingleObject(pinfo.hProcess, 120 * 1000);
	CloseHandle(houtputfile);
	return 0;
}

#endif /*__CYGWIN__*/
