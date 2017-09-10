/*!
 * a virtual camera, using COM
 * @author hongjun.liao <docici@126.com>, @date 2017/9/6
 * */
#ifdef WIN32

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <afxwin.h>   // MFC core and standard components, NOT use in this source file
#include "d:/jun/agent_cli/com_probe.h"

static size_t const BUFLEN = 1024 * 1024;
static char g_recvbuf[BUFLEN];

/* from Com_Proxy::SockThread_Ex */ 
static int test_00_sockaddr_main(int argc, char ** argv)
{
	WSADATA wsaData = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET hSrvSkt = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in stAddr;

	memset(&stAddr, 0, sizeof(stAddr));
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = 0;
	stAddr.sin_port = htons(0);
	int x = bind(hSrvSkt, (const struct sockaddr*)&stAddr, sizeof(stAddr));	
	DWORD dwError = WSAGetLastError();

	sockaddr_in addrMy;
	memset(&addrMy, 0, sizeof(addrMy));
	int len = sizeof(addrMy);
	getsockname(hSrvSkt, (sockaddr*)&addrMy, &len);

	int port = ntohs(addrMy.sin_port);
	fprintf(stdout, "%s: sockaddr_in set to 0, getsockname return: port=%d\n", __FUNCTION__, port);
	
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
	setvbuf(stdout, 0, _IONBF, 0);

	fprintf(stdout, "%s: a virtual camera, using COM, for testing xh/agent, build at %s %s\n", 
		__FUNCTION__, __DATE__, __TIME__);
	/* for test */
	test_00_sockaddr_main(argc, argv);

	PktComProbe comprobe = { MAGIC_COM, RE_COM_PROBE, 0, 0, MAGIC_COM_END};
	comprobe.cksum = in_cksum((unsigned short *)&comprobe, sizeof(char) * 12);

	char file[] = "//./com1";
	HANDLE hdl = CreateFile(file, GENERIC_READ | GENERIC_WRITE, 0, 0, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (hdl == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: CreateFile failed, COM='%s'\n", __FUNCTION__, file);
		return -1;
	}
	
	DWORD const N = sizeof(PktComProbe);
	size_t pos = 0;
	for (;;) {
		char * buf = g_recvbuf + pos;
		
		DWORD m = 0;
		BOOL r = ReadFile(hdl, buf, N, &m, 0);
		if (!r || m < 0) {
			fprintf(stderr, "%s: CreateFile failed, exit\n", __FUNCTION__);
			return -1;
		}

		pos += m;
		fprintf(stdout, "%s: recvdata, len=%d\n", __FUNCTION__, m);

		if (m < N) continue;

		PktComProbe rcomprobe = { 0 };
		memcpy(&rcomprobe, buf, N);

		if (memcmp(&rcomprobe, &comprobe, N) != 0) {
			fprintf(stderr, "%s: PktComProbe unmatch, skip\n", __FUNCTION__);
			pos = 0;
			continue;
		}

		PktComProbeAck comack = { MAGIC_COM, RE_COM_PROBE_ACK, 0 };
		DWORD written = 0;
		r = WriteFile(hdl, &comack, sizeof(comack), &written, 0);
		if (!r || written <= 0) {
			fprintf(stderr, "%s: WriteFile failed, exit\n", __FUNCTION__);
			return -1;
		}

		fprintf(stdout, "%s: recv PktComProbe and PktComProbeAck sent\n", __FUNCTION__);
	}

	return 0;
}

#endif /* WIN32 */
