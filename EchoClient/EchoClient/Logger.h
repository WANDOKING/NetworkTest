// version 2.0.0
#pragma once

#include <iostream>

typedef wchar_t WCHAR;
typedef void* HANDLE;

/************ recommend call this macros instead of Logger::functions ************/
#define LOG(M) Logger::LogMessage((M), __FILEW__, __LINE__)

#define LOG_WITH_WSAERROR(M) do {                 \
	Logger::LogMessage((M), __FILEW__, __LINE__); \
	Logger::LogWSAError();                        \
} while (0)                                       \

#define CRASH() Logger::RaiseCrash()

#define ASSERT_WITH_MESSAGE(C, M) Logger::Assert((C), (M), __FILEW__, __LINE__)

#define ASSERT(C) Logger::Assert((C), L"Assertion Failed!", __FILEW__, __LINE__)

#define LOGF Logger::LogF
/*********************************************************************************/

class Logger
{
public:
	static void LogWSAError();
	static void LogMessage(const WCHAR* message);
	static void LogMessage(const WCHAR* message, const WCHAR* fileName, int line);
	static void Assert(bool condition, const WCHAR* message);
	static void Assert(bool condition, const WCHAR* message, const WCHAR* fileName, int line);

	// formatted string Log
	static void LogF(const WCHAR* formatMessage, ...);

	// intentional crash
	static void RaiseCrash();
private:
	Logger();
	~Logger();

	static void getCurrentTimeInfo(WCHAR* buffer, size_t bufferSize);

	static Logger mInstance;
	static HANDLE mhLogFile;
};