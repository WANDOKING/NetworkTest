#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Logger.h"
#include "WSA.h"

#define REMOTE_IP L"255.255.255.255"
#define REMOTE_PORT 9000
#define BUFFER_SIZE 512

int main(void)
{
	WSA_STARTUP();
	setlocale(LC_ALL, "Korean");

	SOCKET remoteSocket;
	SOCKADDR_IN remoteAddress;

	int retSetsockopt;

	// socket()
	remoteSocket = socket(AF_INET, SOCK_DGRAM, 0);
	ASSERT_WITH_MESSAGE(remoteSocket != INVALID_SOCKET, L"remoteSocket socket() error");

	// broadcast
	BOOL bEnable = TRUE;
	retSetsockopt = setsockopt(remoteSocket, SOL_SOCKET, SO_BROADCAST, (const char*)(&bEnable), sizeof(bEnable));
	ASSERT_WITH_MESSAGE(remoteSocket != INVALID_SOCKET, L"remoteSocket setsockopt() error");

	// serverAddress initialize
	ZeroMemory(&remoteAddress, sizeof(remoteAddress));
	remoteAddress.sin_family = AF_INET;
	InetPtonW(AF_INET, REMOTE_IP, &remoteAddress.sin_addr.s_addr);
	remoteAddress.sin_port = htons(REMOTE_PORT);

	WCHAR buffer[BUFFER_SIZE + 1];
	int length;
	int retSendto;

	while (true)
	{
		// send()
		wprintf(L"\nInput data to send : ");
		if (fgetws(buffer, BUFFER_SIZE + 1, stdin) == nullptr)
		{
			break;
		}

		length = (int)wcslen(buffer);
		if (buffer[length - 1] == L'\n')
		{
			buffer[length - 1] = L'\0';
		}

		if (wcslen(buffer) == 0)
		{
			break;
		}

		retSendto = sendto(remoteSocket, (const char*)buffer, (int)(wcslen(buffer) * sizeof(WCHAR)), 0, (SOCKADDR*)&remoteAddress, sizeof(remoteAddress));
		if (retSendto == SOCKET_ERROR)
		{
			LOG_WITH_WSAERROR(L"remoteSocket sendto() error");
			WSA_CLEANUP();
			CRASH();
		}

		wprintf(L"[UDP Broadcast - sendto : %dbyte]\n", retSendto);
	}

	// closesocket()
	closesocket(remoteSocket);

	WSA_CLEANUP();
	return 0;
}