// version 2.0.0
#pragma once

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "pathcch.lib")

#include <WinSock2.h>
#include <time.h>
#include <PathCch.h>
#include <Windows.h>

#include "Logger.h"

#define LOG_MESSAGE_MAX_LENGTH 512
#define DAY_INFO_BUFFER_LENGTH 16

Logger Logger::mInstance;
HANDLE Logger::mhLogFile;

// recommend call this macro instead of getCurrentTimeInfo()
#define GET_DAY_INFO(B) getCurrentTimeInfo((B), sizeof((B)))

#define WRITE_LOG(M, L) WriteFile(mhLogFile, (M), (DWORD)((L) * sizeof(WCHAR)), nullptr, nullptr)

void Logger::LogWSAError()
{
	WCHAR log[LOG_MESSAGE_MAX_LENGTH * 2];
	size_t logLength = 0;
	bool retWriteFile = false;

	wsprintf(log, L" (ERROR : %6d)", WSAGetLastError());

	logLength = wcslen(log);
	if (logLength > LOG_MESSAGE_MAX_LENGTH)
	{
		LogMessage(L"Logging Error : Log Message is too long", __FILEW__, __LINE__);
		RaiseCrash();
		return;
	}

	retWriteFile = WRITE_LOG(log, logLength);
	if (retWriteFile == false)
	{
		wprintf(L"WriteFile() ERROR : %d", GetLastError());
		RaiseCrash();
	}

	FlushFileBuffers(mhLogFile);
}

void Logger::LogMessage(const WCHAR* message)
{
	WCHAR log[LOG_MESSAGE_MAX_LENGTH * 2];
	WCHAR dayInfo[DAY_INFO_BUFFER_LENGTH];
	size_t logLength = 0;
	bool retWriteFile = false;

	GET_DAY_INFO(dayInfo);
	wsprintf(log, L"\n[%s] : %s", dayInfo, message);

	logLength = wcslen(log);
	if (logLength > LOG_MESSAGE_MAX_LENGTH)
	{
		LogMessage(L"Logging Error : Log Message is too long", __FILEW__, __LINE__);
		RaiseCrash();
		return;
	}

	retWriteFile = WRITE_LOG(log, logLength);
	if (retWriteFile == false)
	{
		wprintf(L"WriteFile() ERROR : %d", GetLastError());
		RaiseCrash();
	}

	FlushFileBuffers(mhLogFile);
}

void Logger::LogMessage(const WCHAR* message, const WCHAR* fileName, int line)
{
	WCHAR log[LOG_MESSAGE_MAX_LENGTH * 2];
	WCHAR dayInfo[DAY_INFO_BUFFER_LENGTH];
	size_t logLength = 0;
	bool retWriteFile = false;

	GET_DAY_INFO(dayInfo);
	wsprintf(log, L"\n[%s][%s line:%4d] : %s", dayInfo, fileName, line, message);

	logLength = wcslen(log);
	if (logLength > LOG_MESSAGE_MAX_LENGTH)
	{
		LogMessage(L"Logging Error : Log Message is too long", __FILEW__, __LINE__);
		RaiseCrash();
		return;
	}

	retWriteFile = WRITE_LOG(log, logLength);
	if (retWriteFile == false)
	{
		wprintf(L"WriteFile() ERROR : %d", GetLastError());
		RaiseCrash();
	}

	FlushFileBuffers(mhLogFile);
}

void Logger::Assert(bool condition, const WCHAR* message)
{
	if (condition == true)
	{
		return;
	}

	LogMessage(message);
	LogWSAError();
	RaiseCrash();
}

void Logger::Assert(bool condition, const WCHAR* message, const WCHAR* fileName, int line)
{
	if (condition == true)
	{
		return;
	}

	LogMessage(message, fileName, line);
	LogWSAError();
	RaiseCrash();
}

void Logger::LogF(const WCHAR* formatMessage, ...)
{
	WCHAR completeMessage[LOG_MESSAGE_MAX_LENGTH * 2];
	WCHAR dayInfo[DAY_INFO_BUFFER_LENGTH];
	size_t logLength = 0;
	bool retWriteFile = false;

	va_list ap;
	va_start(ap, formatMessage);
	{
		GET_DAY_INFO(dayInfo);

		vswprintf(completeMessage, LOG_MESSAGE_MAX_LENGTH * 2, formatMessage, ap);
	}
	va_end(ap);

	LogMessage(completeMessage);
}

void Logger::getCurrentTimeInfo(WCHAR* buffer, size_t bufferSize)
{
	if (bufferSize != DAY_INFO_BUFFER_LENGTH * 2)
	{
		wprintf(L"getCurrentTimeInfo() ERROR : dayInfo buffer length is %d\n", (int)bufferSize);
		RaiseCrash();
	}

	time_t startTime = time(nullptr);
	tm localTime;
	localtime_s(&localTime, &startTime);

	int YYYY = localTime.tm_year + 1900;
	int MM = localTime.tm_mon + 1;
	int DD = localTime.tm_mday;
	int hh = localTime.tm_hour;
	int mm = localTime.tm_min;
	int ss = localTime.tm_sec;
	wsprintfW(buffer, L"%04d%02d%02d_%02d%02d%02d", YYYY, MM, DD, hh, mm, ss);
}

#pragma warning(push)
#pragma warning(disable: 6011)
void Logger::RaiseCrash()
{
	int* nullPointer = 0x00000000;
	*nullPointer = 0;
}
#pragma warning(pop)

Logger::Logger()
{
	WCHAR fileName[_MAX_PATH * 2];

	// get logFileName
	WCHAR dayInfo[DAY_INFO_BUFFER_LENGTH];
	GET_DAY_INFO(dayInfo);

	// get process path
	DWORD dwPID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
	WCHAR processPath[MAX_PATH];
	if (GetModuleFileNameW(nullptr, processPath, MAX_PATH) == 0)
	{
		CloseHandle(hProcess);
		exit(1);
	}
	CloseHandle(hProcess);

	wsprintfW(fileName, L"%s Log", processPath);
	CreateDirectoryW(fileName, nullptr); // create directory

	wsprintfW(processPath, L"%s", fileName); // path update (log file directory)
	wsprintfW(fileName, L"%s\\Log_%s.txt", processPath, dayInfo); // log file path

	mhLogFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mhLogFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile() Failed %d\n", GetLastError());
		RaiseCrash();
	}

	// Write BOM
	unsigned short BOM_UTF_16_LE = 0xFEFF;
	WriteFile(mhLogFile, &BOM_UTF_16_LE, sizeof(BOM_UTF_16_LE), nullptr, nullptr);
}

Logger::~Logger()
{
	CloseHandle(mhLogFile);
}