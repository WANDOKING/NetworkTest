#pragma comment(lib, "ws2_32")

#include "WSA.h"
#include "Logger.h"

#include <windows.h>
#include <iostream>
#include <list>
#include <WS2tcpip.h>

#define SERVER_PORT 13000

#define BUFFER_LENGTH 4096
#define MAX_THREAD_COUNT 4

struct Session
{
	SOCKET Socket;
	WCHAR buffer[BUFFER_LENGTH];
};

CRITICAL_SECTION g_cs;
std::list<SOCKET> g_clientSockets;
SOCKET g_listenSocket;
HANDLE g_hIOCP;

void SendBroadcast(WCHAR* message, int messageLength);
void CloseClient(SOCKET clientSocket);

DWORD WINAPI ThreadComplete(LPVOID pParam);
DWORD WINAPI ThreadAccept(LPVOID pParam);

int main(void)
{
	WSA_STARTUP();
	setlocale(LC_ALL, "Korean");
	InitializeCriticalSection(&g_cs);

	int retBind;
	int retListen;

	// IOCP 생성
	g_hIOCP = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE, // 연결된 파일 없음
		nullptr,              // 기존 핸들 없음
		0,                    // 식별자(Key) 해당되지 않음
		0);                   // 스레드 개수는 OS에 맡김
	ASSERT_WITH_MESSAGE(g_hIOCP != nullptr, L"CreateIOCP() Error");

	// IOCP 스레드 생성
	HANDLE hThread;
	for (int i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		hThread = CreateThread(nullptr, 0, ThreadComplete, nullptr, 0, nullptr);
		CloseHandle(hThread);
	}

	g_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
	ASSERT_WITH_MESSAGE(g_listenSocket != INVALID_SOCKET, L"listenSocket WSASocket() Error");

	// bind()
	SOCKADDR_IN serverAddress = { 0, };
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(SERVER_PORT);
	retBind = bind(g_listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	ASSERT_WITH_MESSAGE(retBind != SOCKET_ERROR, L"listenSocket bind() Error");

	// listen()
	retListen = listen(g_listenSocket, SOMAXCONN);
	ASSERT_WITH_MESSAGE(retListen != SOCKET_ERROR, L"listenSocket listen() Error");

	hThread = CreateThread(nullptr, 0, ThreadAccept, nullptr, 0, nullptr);
	CloseHandle(hThread);

	wprintf(L"Chat Server Start - PORT:%d\n", SERVER_PORT);

	// main()이 반환하지 않도록 하기 위함
	while (true)
	{
		int ignore = getchar();
	}

	DeleteCriticalSection(&g_cs);
	WSA_CLEANUP();
	return 0;
}

void SendBroadcast(WCHAR* message, int messageSize)
{
	EnterCriticalSection(&g_cs);
	int retSend;
	{
		for (auto it = g_clientSockets.begin(); it != g_clientSockets.end(); ++it)
		{
			SOCKET visit = *it;
			retSend = send(visit, (char*)message, messageSize, 0);
		}
	}
	LeaveCriticalSection(&g_cs);
}

void CloseClient(SOCKET clientSocket)
{
	closesocket(clientSocket);

	EnterCriticalSection(&g_cs);
	{
		g_clientSockets.remove(clientSocket);
	}
	LeaveCriticalSection(&g_cs);
}

DWORD WINAPI ThreadComplete(LPVOID pParam)
{
	DWORD dwTransferredSize = 0;
	DWORD dwFlag = 0;
	Session* session = nullptr;
	LPWSAOVERLAPPED wsaOverlapped = nullptr;
	bool bResult;

	int retWSARecv;

	wprintf(L"[IOCP Worker Thread Start]\n");

	while (true)
	{
		bResult = GetQueuedCompletionStatus(
			g_hIOCP,               // Dequeue할 IOCP 핸들
			&dwTransferredSize,    // 수신한 데이터 크기
			(PULONG_PTR)&session,  // 수신된 데이터가 저장된 메모리
			&wsaOverlapped,        // OVERLAPPED 구조체
			INFINITE);             // 이벤트를 무한 대기

		// 비정상적인 상황
		if (bResult == false)
		{
			if (wsaOverlapped == nullptr)
			{
				wprintf(L"GQCS : IOCP handle closed\n");
				break;
			}
			
			if (session != nullptr)
			{
				CloseClient(session->Socket);
				delete wsaOverlapped;
				delete session;
			}

			wprintf(L"GQCS : server closed or abnormal disconnect\n");
			continue;
		}
		
		if (dwTransferredSize == 0)
		{
			CloseClient(session->Socket);
			delete wsaOverlapped;
			delete session;
			wprintf(L"client send 0 - disconnected\n");
			continue;
		}

		SendBroadcast(session->buffer, dwTransferredSize);
		memset(session->buffer, 0, sizeof(session->buffer));

		// 다시 IOCP에 등록
		DWORD dwRecvSize = 0;
		DWORD dwFlag = 0;
		WSABUF wsaBuffer;
		wsaBuffer.buf = (CHAR*)(session->buffer);
		wsaBuffer.len = sizeof(session->buffer);

		retWSARecv = WSARecv(session->Socket, &wsaBuffer, 1, &dwRecvSize, &dwFlag, wsaOverlapped, nullptr);
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			wprintf(L"WSARecv() != WSA_IO_PENDING Error\n");
		}
	}
	wprintf(L"[IOCP Worker Thread End]\n");
	return 0;
}

DWORD WINAPI ThreadAccept(LPVOID pParam)
{
	SOCKET acceptedClientSocket;
	SOCKADDR_IN clientAddress;
	int clientAddressLength = sizeof(clientAddress);
	int retWSARecv;

	while (true)
	{
		// accept()
		acceptedClientSocket = accept(g_listenSocket, (SOCKADDR*)&clientAddress, &clientAddressLength);
		if (acceptedClientSocket == INVALID_SOCKET)
		{
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)
			{
				continue;
			}
			else
			{
				LOG_WITH_WSAERROR(L"listenSocket accept() error");
				CRASH();
				return 1;
			}
		}

		// log new connected client
		{
			WCHAR clientIp[16] = { 0, };
			int clientPort = ntohs(clientAddress.sin_port);
			InetNtopW(AF_INET, &clientAddress, clientIp, 16);
			wprintf(L"client connected - %s:%d\n", clientIp, clientPort);
		}
		
		// insert clientSockets
		EnterCriticalSection(&g_cs);
		{
			g_clientSockets.push_back(acceptedClientSocket);
		}
		LeaveCriticalSection(&g_cs);

		// session create & attach to IOCP
		Session* newSession = new Session;
		ZeroMemory(newSession, sizeof(Session));
		newSession->Socket = acceptedClientSocket;

		LPWSAOVERLAPPED wsaOverlapped = new WSAOVERLAPPED;
		ZeroMemory(wsaOverlapped, sizeof(WSAOVERLAPPED));

		CreateIoCompletionPort((HANDLE)acceptedClientSocket, g_hIOCP, (ULONG_PTR)newSession, 0);

		// 클라이언트가 보낸 정보를 비동기 수신
		DWORD dwRecvSize = 0;
		DWORD dwFlag = 0;
		WSABUF wsaBuffer;
		wsaBuffer.buf = (CHAR*)(newSession->buffer);
		wsaBuffer.len = sizeof(newSession->buffer);
		
		// 여기 아직 완벽히 이해 못했음 !!!
		// 여기 아직 완벽히 이해 못했음 !!!
		// 여기 아직 완벽히 이해 못했음 !!!
		retWSARecv = WSARecv(acceptedClientSocket, &wsaBuffer, 1, &dwRecvSize, &dwFlag, wsaOverlapped, nullptr);
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			wprintf(L"WSARecv() != WSA_IO_PENDING Error\n");
		}
	}

	return 0;
}