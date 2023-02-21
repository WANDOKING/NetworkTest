#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Logger.h"
#include "WSA.h"

#define LOCAL_PORT 9000
#define BUFFER_SIZE 512

int main(void)
{
	WSA_STARTUP();
	setlocale(LC_ALL, "Korean");

	SOCKET localSocket;
	SOCKADDR_IN localAddress;

	int retBind;

	// socket()
	localSocket = socket(AF_INET, SOCK_DGRAM, 0);
	ASSERT_WITH_MESSAGE(localSocket != INVALID_SOCKET, L"localSocket socket() error");

	// localAddress initialize
	ZeroMemory(&localAddress, sizeof(localAddress));
	localAddress.sin_family = AF_INET;
	localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddress.sin_port = htons(LOCAL_PORT);

	// bind()
	retBind = bind(localSocket, (SOCKADDR*)&localAddress, sizeof(localAddress));
	ASSERT_WITH_MESSAGE(retBind != SOCKET_ERROR, L"localSocket bind() error");

	WCHAR buffer[BUFFER_SIZE + 1];
	int retRecvfrom;

	while (true)
	{
		SOCKADDR_IN peerAddress;
		int peerAddressLength = sizeof(peerAddress);

		// recvfrom()
		retRecvfrom = recvfrom(localSocket, (char*)buffer, BUFFER_SIZE * sizeof(WCHAR), 0, (SOCKADDR*)&peerAddress, &peerAddressLength);
		if (retRecvfrom == SOCKET_ERROR)
		{
			LOG_WITH_WSAERROR(L"localSocket recvfrom() error");
			WSA_CLEANUP();
			CRASH();
		}

		buffer[retRecvfrom / 2] = L'\0';

		WCHAR peerIpAddress[16] = { 0, };
		InetNtopW(AF_INET, &peerAddress, peerIpAddress, 16);
		wprintf(L"\n[Recevied from %s:%d] : %s", peerIpAddress, htons(peerAddress.sin_port), buffer);
	}

	// closesocket()
	closesocket(localSocket);

	WSA_CLEANUP();
	return 0;
}