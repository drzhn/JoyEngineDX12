#include "ThreadManager.h"

namespace JoyEngine
{
	void ThreadManager::Stop()
	{
		m_worker.join();
	}
}
