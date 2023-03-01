// version 1.2.1
#pragma once

#include <iostream>

typedef wchar_t WCHAR;

#define LOG(M) Logger::LogMessage((M), __FILE__, __LINE__)
#define LOG_WITH_WSAERROR(M) Logger::LogMessageWithWSAError((M), __FILE__, __LINE__)
#define CRASH() Logger::RaiseCrash()
#define ASSERT_WITH_MESSAGE(C, M) Logger::Assert((C), (M), __FILE__, __LINE__)
#define ASSERT(C) Logger::Assert((C), L"Assertion Failed!", __FILE__, __LINE__)
#define LOGF Logger::LogF

class Logger
{
public:
	static void LogMessage(const WCHAR* message);
	static void LogMessage(const WCHAR* message, const char* fileName, int line);
	static void LogMessageWithWSAError(const WCHAR* message);
	static void LogMessageWithWSAError(const WCHAR* message, const char* fileName, int line);
	static void Assert(bool condition, const WCHAR* message);
	static void Assert(bool condition, const WCHAR* message, const char* fileName, int line);

	// formatted string Log
	static void LogF(const WCHAR* formatMessage, ...);

	// intentional crash
	static void RaiseCrash();
private:
	Logger();
	~Logger();

	static void getCurrentTimeInfo(WCHAR* buffer);

	static Logger mInstance;
	static FILE* mLogFile;
};