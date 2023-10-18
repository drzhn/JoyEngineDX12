#include "SceneManager.h"

#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	void SceneManager::Init()
	{
		TIME_PERF("SceneManager init")

		m_scene = std::make_unique<Scene>("scenes/room.json");
	}

	void SceneManager::Start()
	{
	}

	void SceneManager::Stop()
	{
		m_scene = nullptr;
	}

	void SceneManager::Update()
	{
		m_scene->Update();
	}

	SceneManager::~SceneManager()
	{
		if (m_scene != nullptr)
		{
			m_scene = nullptr;
		}
	}
}
