#include "SceneManager.h"

#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	void SceneManager::Init()
	{
		TIME_PERF("SceneManager init");
		m_scene = m_sceneTree.Create<Scene>("scenes/test_scene.scene");
		m_transformProvider.Init();
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
		m_transformProvider.Update();
	}

	SceneManager::~SceneManager()
	{
		if (m_scene != nullptr)
		{
			m_scene = nullptr;
		}
	}
}
