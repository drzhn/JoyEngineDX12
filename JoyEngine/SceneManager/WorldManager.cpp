#include "WorldManager.h"

#include <rapidjson/document.h>

#include "DataManager/DataManager.h"
#include "RenderManager/BasicRenderer/BasicRenderer.h"
#include "RenderManager/RaytracedDDGIRenderer/RaytracedDDGIRenderer.h"
#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	WorldManager::WorldManager(HWND gameWindowHandle)
	{
		m_renderManager = std::make_unique<BasicRenderer>(gameWindowHandle);
		m_transformProvider = std::make_unique<TransformProvider>(m_renderManager->GetFrameCount());
	}

	void WorldManager::Init()
	{
		TIME_PERF("WorldManager init");
		rapidjson::Document json = DataManager::Get()->GetSerializedData(
			"scenes/test_scene.scene",
			AssetType::World
		);
		m_skybox = std::make_unique<Skybox>(json["skybox"]["texture"].GetString());

		// creating render resources
		m_renderManager->Init(m_skybox.get());

		m_scene = m_sceneTree.Create<Scene>(json["scene"]);
		m_transformProvider->Init();
	}

	void WorldManager::Start()
	{
		m_renderManager->Start();
	}


	void WorldManager::Update()
	{
		m_renderManager->PreUpdate();

		m_scene->Update();
		m_transformProvider->Update();

		m_renderManager->Update();
	}


	void WorldManager::Stop()
	{
		m_renderManager->Stop();

		m_sceneTree.Clear();
	}

	WorldManager::~WorldManager()
	{
		m_renderManager = nullptr;
	}
}
