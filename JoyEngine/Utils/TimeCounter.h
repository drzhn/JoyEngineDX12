#ifndef TIME_COUNTER_H
#define TIME_COUNTER_H
#include <chrono>

#include <Windows.h>

#define TIME_PERF(message) TimeCounter counter = TimeCounter(message);

namespace JoyEngine
{
	class TimeCounter
	{
	public:
		TimeCounter(const char* message) :
			m_message(message)
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}
		~TimeCounter()
		{
			const auto currentTime = std::chrono::high_resolution_clock::now();
			const double time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - m_startTime).count();
			OutputDebugStringA(m_message);
			char buf[16];
			sprintf_s(buf, ": %.3f\n", time);
			OutputDebugStringA(buf);
		}
	private:
		std::chrono::time_point<std::chrono::steady_clock> m_startTime;
		const char* m_message;
	};
}
#endif // TIME_COUNTER_H
