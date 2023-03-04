#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "Logger.h"
#include "WSA.h"

#define BUFFER_SIZE 512
#define BACKLOG_SIZE SOMAXCONN

int main(void)
{
	WSA_STARTUP();
	setlocale(LC_ALL, "Korean");

	SOCKET listenSocket;
	SOCKADDR_IN serverAddress;

	int retBind;
	int retListen;
	int userInputPort;

	// Port Number Input
	wprintf(L"Port Number : ");
	scanf_s("%d", &userInputPort);
	int ignore = getchar();
	ASSERT_WITH_MESSAGE(userInputPort >= 0 && userInputPort <= USHRT_MAX, L"Port Number error");

	// socket()
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_WITH_MESSAGE(listenSocket != INVALID_SOCKET, L"listenSocket socket() error");

	// serverAddress initialize
	serverAddress;
	ZeroMemory(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons((unsigned short)userInputPort);

	// bind()
	retBind = bind(listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	ASSERT_WITH_MESSAGE(retBind != SOCKET_ERROR, L"listenSocket bind() error");

	// listen()
	retListen = listen(listenSocket, BACKLOG_SIZE);
	ASSERT_WITH_MESSAGE(retListen != SOCKET_ERROR, L"listenSocket listen() error");

	wprintf(L"[Iterative Echo Server Open]\n");
	wprintf(L"Port : %d\n", userInputPort);

	while (true)
	{
		SOCKET clientSocket;
		SOCKADDR_IN clientAddress;
		int clientAddressLength = sizeof(clientAddress);
		WCHAR buffer[BUFFER_SIZE + 1];

		int retRecv;
		int retSend;

		wprintf(L"Listening...\n");

		// accept()
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddress, &clientAddressLength);
		ASSERT_WITH_MESSAGE(clientSocket != INVALID_SOCKET, L"listenSocket accept() error");

		// Get Client IP, Address
		WCHAR clientIpAddress[16] = { 0 };
		InetNtopW(AF_INET, &clientAddress.sin_addr, clientIpAddress, 16);
		wprintf(L"\n[client connected : IP = %s, PORT = %d]\n", clientIpAddress, ntohs(clientAddress.sin_port));

		// Echo Service
		while (true)
		{
			// recv()
			retRecv = recv(clientSocket, (char*)buffer, BUFFER_SIZE * sizeof(WCHAR), 0);
			if (retRecv == SOCKET_ERROR)
			{
				int errorCode = WSAGetLastError();
				if (errorCode == WSAECONNRESET)
				{
					break;
				}
				else
				{
					LOG_WITH_WSAERROR(L"recv() error");
					WSA_CLEANUP();
					CRASH();
				}
			}

			wprintf(L"[recv : %dbytes]\n", retRecv);

			if (retRecv == 0)
			{
				break;
			}

			buffer[retRecv / sizeof(WCHAR)] = L'\0';
			wprintf(L"received : %s\n", buffer);

			retSend = send(clientSocket, (char*)buffer, retRecv, 0);
			if (retSend == SOCKET_ERROR)
			{
				LOG_WITH_WSAERROR(L"send() error");
				WSA_CLEANUP();
				CRASH();
			}

			wprintf(L"[send : %dbytes]\n", retSend);
		}

		// closesocket()
		closesocket(clientSocket);
		wprintf(L"[client disconnected : IP = %s, PORT = %d]\n\n", clientIpAddress, ntohs(clientAddress.sin_port));
	}

	// closesocket()
	closesocket(listenSocket);

	WSA_CLEANUP();
	return 0;
}