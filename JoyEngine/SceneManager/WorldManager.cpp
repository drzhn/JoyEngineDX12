#include "WorldManager.h"

#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	void WorldManager::Init()
	{
		TIME_PERF("WorldManager init");
		m_scene = m_sceneTree.Create<Scene>("scenes/test_scene.scene");
		m_transformProvider.Init();
	}

	void WorldManager::Start()
	{
	}

	void WorldManager::Stop()
	{
		m_scene = nullptr;
	}

	void WorldManager::Update()
	{
		m_scene->Update();
		m_transformProvider.Update();
	}

	WorldManager::~WorldManager()
	{
		if (m_scene != nullptr)
		{
			m_scene = nullptr;
		}
	}
}
