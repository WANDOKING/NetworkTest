// version 1.2.0
#pragma once
#pragma comment(lib, "pathcch.lib")

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <time.h>
#include <cassert>
#include <PathCch.h>

#include "Logger.h"

#define FILE_NAME_MAX_LENGTH 1024

Logger Logger::mInstance;
FILE* Logger::mLogFile;

void Logger::LogMessage(const WCHAR* message)
{
	WCHAR dayInfo[FILE_NAME_MAX_LENGTH];
	getCurrentTimeInfo(dayInfo);
	fwprintf(mLogFile, L"[%s] : %s\n", dayInfo, message);
	fflush(mLogFile);
}

void Logger::LogMessage(const WCHAR* message, const char* fileName, int line)
{
	WCHAR dayInfo[FILE_NAME_MAX_LENGTH];
	getCurrentTimeInfo(dayInfo);

	WCHAR wFileName[FILE_NAME_MAX_LENGTH];
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, fileName, -1, wFileName, FILE_NAME_MAX_LENGTH);

	fwprintf(mLogFile, L"[%s][%s line:%4d] : %s\n", dayInfo, wFileName, line, message);
	fflush(mLogFile);
}

void Logger::LogMessageWithWSAError(const WCHAR* message)
{
	WCHAR dayInfo[FILE_NAME_MAX_LENGTH];
	getCurrentTimeInfo(dayInfo);

	fwprintf(mLogFile, L"[%s] : %s (Error : %d)\n", dayInfo, message, WSAGetLastError());
	fflush(mLogFile);
}

void Logger::LogMessageWithWSAError(const WCHAR* message, const char* fileName, int line)
{
	WCHAR dayInfo[FILE_NAME_MAX_LENGTH];
	getCurrentTimeInfo(dayInfo);

	WCHAR wFileName[FILE_NAME_MAX_LENGTH];
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, fileName, -1, wFileName, FILE_NAME_MAX_LENGTH);

	fwprintf(mLogFile, L"[%s][%s line:%4d] : %s (Error : %d)\n", dayInfo, wFileName, line, message, WSAGetLastError());
	fflush(mLogFile);
}

void Logger::getCurrentTimeInfo(WCHAR* buffer)
{
	time_t startTime = time(nullptr);
	tm localTime;
	localtime_s(&localTime, &startTime);

	int YYYY = localTime.tm_year + 1900;
	int MM = localTime.tm_mon + 1;
	int DD = localTime.tm_mday;
	int hh = localTime.tm_hour;
	int mm = localTime.tm_min;
	int ss = localTime.tm_sec;
	wsprintf(buffer, L"%04d%02d%02d_%02d%02d%02d", YYYY, MM, DD, hh, mm, ss);
}

Logger::Logger()
{
	WCHAR buffer[FILE_NAME_MAX_LENGTH];

	// get logFileName
	WCHAR dayInfo[16];
	getCurrentTimeInfo(dayInfo);

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

	wsprintfW(buffer, L"%s Log", processPath);
	CreateDirectoryW(buffer, nullptr); // create directory

	wsprintfW(processPath, L"%s", buffer); // path update (log file directory)
	wsprintfW(buffer, L"%s\\Log_%s.txt", processPath, dayInfo); // log file path

	if (_wfopen_s(&mLogFile, buffer, L"w") == EINVAL)
	{
		printf("%d\n", GetLastError());
		assert(false);
		RaiseCrash();
	}
}

Logger::~Logger()
{
	fclose(mLogFile);
}

#pragma warning(push)
#pragma warning(disable: 6011)

void Logger::RaiseCrash()
{
	int* nullPointer = 0x00000000;
	*nullPointer = 0;
}

#pragma warning(pop)

void Logger::Assert(bool condition, const WCHAR* message)
{
	if (condition == true)
	{
		return;
	}

	LogMessage(message);
	RaiseCrash();
}

void Logger::Assert(bool condition, const WCHAR* message, const char* fileName, int line)
{
	if (condition == true)
	{
		return;
	}

	LogMessage(message, fileName, line);
	RaiseCrash();
}

void Logger::LogF(const WCHAR* formatMessage, ...)
{
	va_list ap;
	va_start(ap, formatMessage);

	WCHAR dayInfo[FILE_NAME_MAX_LENGTH];
	getCurrentTimeInfo(dayInfo);

	fwprintf(mLogFile, L"[%s] : ", dayInfo);
	vfwprintf(mLogFile, formatMessage, ap);
	fwprintf(mLogFile, L"\n");

	fflush(mLogFile);

	va_end(ap);
}