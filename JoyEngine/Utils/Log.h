#ifndef LOG_H
#define LOG_H

class Logger
{
public:
	static void Log(const char* message);
	static void LogFormat(const char* format...);
};
#endif // LOG_H
