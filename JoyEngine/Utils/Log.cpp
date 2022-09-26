#include "Log.h"

#include <cstdio>

#include "Windows.h"

#define MAX_LOG_SIZE 1000

char g_logData[MAX_LOG_SIZE];

void Logger::Log(const char* message)
{
	_snprintf_s(g_logData, MAX_LOG_SIZE, MAX_LOG_SIZE, "%s", message);
	OutputDebugStringA(g_logData);
}

void Logger::LogFormat(const char* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vsnprintf_s(g_logData, MAX_LOG_SIZE, MAX_LOG_SIZE, format, argptr);
	va_end(argptr);

	OutputDebugStringA(g_logData);
}
