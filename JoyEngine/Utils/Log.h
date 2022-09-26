#ifndef LOG_H
#define LOG_H
#include <cstdint>

class Logger
{
public:
	static void Log(const char* message);
	static void LogFormat(const char* format...);
	static void LogUintArray(uint32_t* array, size_t size, uint32_t count = 1024);
};
#endif // LOG_H
