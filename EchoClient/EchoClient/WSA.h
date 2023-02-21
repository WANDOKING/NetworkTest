#pragma once

#include <WinSock2.h>

#define WSA_STARTUP()                          \
do                                             \
{                                              \
	WSADATA wsa;                               \
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) \
	{                                          \
		return 1;                              \
	}                                          \
} while(false)                                 \

#define WSA_CLEANUP() WSACleanup()