#ifndef TIME_H
#define TIME_H

#include <chrono>
#include <functional>
namespace JoyEngine {

	class Time
	{
	public:
		static void Init(std::function<void (float currentDeltaTime)>) noexcept;
		static void Update() noexcept;
		static float GetDeltaTime() noexcept;
		static float GetTime() noexcept;
		static float GetFrameCount() noexcept;
	private:
		static constexpr float m_deltaTimeCounterDelay = 0.3f;
		static float m_timeFromLastDeltaTimeCounter;
		static uint32_t m_framesFromLastDeltaTimeCounter;

		static float m_deltaTime;
		static float m_time;
		static uint32_t m_frameCount;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_prevUpdateTime;
		static std::chrono::time_point<std::chrono::high_resolution_clock> m_curUpdateTime;

		static std::function<void(float currentDeltaTime)> m_deltaTimeHandler;
	};
}
#endif //TIME_H

