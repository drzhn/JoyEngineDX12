#include "JoyEngine.h"

#include <string>

#include "JoyContext.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/RenderManager.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "DataManager/DataManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Common/Time.h"
#include "InputManager/InputManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	JoyEngine::JoyEngine(HINSTANCE instance, HWND windowHandle) :
		m_windowHandle(windowHandle),
		m_inputManager(new InputManager()),
		m_graphicsContext(new GraphicsManager(instance, windowHandle)),
		m_memoryManager(new MemoryManager()),
		m_dataManager(new DataManager()),
		m_descriptorSetManager(new DescriptorManager()),
		m_resourceManager(new ResourceManager()),
		m_sceneManager(new SceneManager()),
		m_renderManager(new RenderManager())
	{
		ASSERT(m_inputManager != nullptr);
		ASSERT(m_graphicsContext != nullptr);
		ASSERT(m_memoryManager != nullptr);
		ASSERT(m_dataManager != nullptr);
		ASSERT(m_descriptorSetManager != nullptr);
		ASSERT(m_resourceManager != nullptr);
		ASSERT(m_sceneManager != nullptr);
		ASSERT(m_renderManager != nullptr);

		JoyContext::Init(
			m_inputManager.get(),
			m_graphicsContext.get(),
			m_memoryManager.get(),
			m_dataManager.get(),
			m_descriptorSetManager.get(),
			m_resourceManager.get(),
			m_sceneManager.get(),
			m_renderManager.get()
		);

		OutputDebugStringA("Context created\n");
	}

	void JoyEngine::Init() const noexcept
	{
		Time::Init(m_deltaTimeHandler);

		m_memoryManager->Init();
		m_descriptorSetManager->Init();
		m_renderManager->Init();
		m_sceneManager->Init();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		std::string s = "Context initialized in " + std::to_string(time) + " seconds\n";
		OutputDebugStringA(s.c_str());
	}

	void JoyEngine::Start() const noexcept
	{
		m_renderManager->Start();
	}

	void JoyEngine::Update() const noexcept
	{
		Time::Update();

		//m_memoryManager->Update();
		m_sceneManager->Update();
		m_renderManager->Update();
	}

	void JoyEngine::Stop() const noexcept
	{
		m_renderManager->Stop();
	}

	JoyEngine::~JoyEngine()
	{
		Stop();
		// will destroy managers in certain order
		m_inputManager = nullptr;
		m_sceneManager = nullptr; // unregister mesh renderers, remove descriptor set, pipelines, pipeline layouts
		m_resourceManager = nullptr; //delete all scene render data (buffers, textures)
		m_renderManager = nullptr; //delete swapchain, synchronisation, framebuffers
		m_descriptorSetManager = nullptr;
		m_dataManager = nullptr;
		m_memoryManager = nullptr; //free gpu memory
		m_graphicsContext = nullptr; //delete surface, device, instance
		OutputDebugStringA("Context destroyed\n");
	}

	void JoyEngine::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_inputManager->HandleWinMessage(hwnd, uMsg, wParam, lParam);
	}
}
