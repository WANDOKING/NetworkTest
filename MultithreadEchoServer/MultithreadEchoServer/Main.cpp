#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WinSock2.h>

#define SERVER_PORT 7500
#define BUFFER_SIZE 512

DWORD EchoService(LPVOID clientSocket);

int main(void)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 0;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN serverAddress = { 0, };
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(SERVER_PORT);

	bind(listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));

	listen(listenSocket, SOMAXCONN);

	while (true)
	{
		SOCKADDR_IN clientAddress;
		int clientAddressLength = sizeof(clientAddress);

		SOCKET acceptedSocket = accept(listenSocket, (SOCKADDR*)&clientAddress, &clientAddressLength);

		CreateThread(nullptr, 0, EchoService, (LPVOID)acceptedSocket, 0, 0);
	}

	Sleep(10000);
	WSACleanup();
}

DWORD EchoService(LPVOID sock)
{
	SOCKET clientSocket = (SOCKET)sock;
	WCHAR buffer[BUFFER_SIZE];

	int retRecv;
	int retSend;

	while (true)
	{
		retRecv = recv(clientSocket, (char*)buffer, BUFFER_SIZE, 0);
		if (retRecv == SOCKET_ERROR || retRecv == 0)
		{
			break;
		}

		buffer[retRecv / 2] = L'\0';

		wprintf(L"recv %d bytes : %s\n", retRecv, buffer);

		retSend = send(clientSocket, (char*)buffer, retRecv, 0);
		if (retSend == SOCKET_ERROR)
		{
			break;
		}

		wprintf(L"send %d bytes : %s\n", retSend, buffer);
	}

	wprintf(L"disconnected\n");
	closesocket(clientSocket);

	return 0;
}