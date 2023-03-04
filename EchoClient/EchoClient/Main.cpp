#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <wchar.h>

#include "Logger.h"
#include "WSA.h"

#define BUFFER_SIZE 512

//#define USE_CLIENT_BIND
#define CLIENT_PORT 8600

int recvn(SOCKET sock, char* buffer, int length, int flags);

int main(void)
{
	WSA_STARTUP();
	setlocale(LC_ALL, "Korean");

	int retConnect;

	WCHAR userInputIp[16];
	int userInputPort;
	wprintf(L"Input IP : ");
	wscanf_s(L"%s", userInputIp, 16);
	wprintf(L"Input Port : ");
	wscanf_s(L"%d", &userInputPort);
	int ignore = getchar();

	// socket()
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_WITH_MESSAGE(clientSocket != INVALID_SOCKET, L"clientSocket socket() error");

	// client socket bind()
#ifdef USE_CLIENT_BIND
	int retBind;
	SOCKADDR_IN clientAddress;
	ZeroMemory(&clientAddress, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = INADDR_ANY;
	clientAddress.sin_port = htons(CLIENT_PORT);

	retBind = bind(clientSocket, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
	ASSERT_WITH_MESSAGE(retBind != SOCKET_ERROR, L"clientSocket bind() error");
#endif

	// serverAddress initialize
	SOCKADDR_IN serverAddress;
	ZeroMemory(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	InetPton(AF_INET, userInputIp, &serverAddress.sin_addr);
	serverAddress.sin_port = htons(userInputPort);

	// connect()
	retConnect = connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	ASSERT_WITH_MESSAGE(retConnect != SOCKET_ERROR, L"clientSocket connect() error");

	wprintf(L"[Connected to %s:%d]\n", userInputIp, userInputPort);

	WCHAR sendBuffer[BUFFER_SIZE + 1];
	WCHAR recvBuffer[BUFFER_SIZE + 1];
	size_t length;

	while (true)
	{
		int retSend;
		int retRecv;

		// send()
		wprintf(L"\nInput data to send : ");
		if (fgetws(sendBuffer, BUFFER_SIZE + 1, stdin) == nullptr)
		{
			break;
		}

		length = wcslen(sendBuffer);
		if (sendBuffer[length - 1] == L'\n')
		{
			sendBuffer[length - 1] = L'\0';
		}

		if (wcslen(sendBuffer) == 0)
		{
			break;
		}

		retSend = send(clientSocket, (const char*)sendBuffer, (int)(wcslen(sendBuffer) * sizeof(WCHAR)), 0);
		if (retSend == SOCKET_ERROR)
		{
			LOG_WITH_WSAERROR(L"clientSocket send() error");
			WSA_CLEANUP();
			CRASH();
		}

		wprintf(L"[send : %dbyte]\n", retSend);

		// recv()
		retRecv = recvn(clientSocket, (char*)recvBuffer, retSend, 0);
		if (retRecv == SOCKET_ERROR)
		{
			LOG_WITH_WSAERROR(L"clientSocket recv() error");
			WSA_CLEANUP();
			CRASH();
		}

		if (retRecv != retSend)
		{
			LOG(L"보낸 데이터와 받은 데이터의 크기가 일치하지 않음");
			WSA_CLEANUP();
			CRASH();
		}

		recvBuffer[retRecv / sizeof(WCHAR)] = L'\0';
		wprintf(L"[recv : %dbytes]\n", retRecv);
		wprintf(L"[recv data] : %s\n", recvBuffer);

		if (wcscmp(sendBuffer, recvBuffer) != 0)
		{
			LOG(L"보낸 데이터와 받은 데이터가 일치하지 않음");
			WSA_CLEANUP();
			CRASH();
		}
	}

	// closesocket()
	closesocket(clientSocket);
	WSA_CLEANUP();
}

int recvn(SOCKET sock, char* buffer, int length, int flags)
{
	int retRecv;
	int left = length;
	char* ptrBuffer = buffer;

	while (left > 0)
	{
		retRecv = recv(sock, ptrBuffer, left, flags);
		if (retRecv == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}

		if (retRecv == 0)
		{
			break;
		}

		left -= retRecv;
		ptrBuffer += retRecv;
	}

	return (length - left);
}