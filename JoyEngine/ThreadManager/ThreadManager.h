#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H
#include <functional>
#include <thread>

namespace JoyEngine
{
	class ThreadManager
	{
	public:
		template <typename... Args>
		void StartTask(Args&&... args)
		{
			m_worker = std::thread(std::forward<Args>(args)...);
		}


		void Stop();

	private:
		std::thread m_worker;
	};
}
#endif // THREAD_MANAGER_H
