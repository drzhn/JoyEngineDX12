#include "SceneManager.h"


namespace JoyEngine {
	void SceneManager::Init()
	{
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
		if (m_scene != nullptr) {
			m_scene = nullptr;
		}
	}
}
