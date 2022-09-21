#include "SceneManager.h"

#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	IMPLEMENT_SINGLETON(SceneManager)

	void SceneManager::Init()
	{
		TIME_PERF("SceneManager init")

		m_scene = std::make_unique<Scene>(GUID::StringToGuid("11dcfeba-c2b6-4c2e-a3c7-51054ff06f1d"));
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
