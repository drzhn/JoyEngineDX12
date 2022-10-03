#include "JoyEngine.h"

#include <string>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"

#include "SceneManager/SceneManager.h"
#include "RenderManager/IRenderManager.h"
#include "RenderManager/RenderManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/ResourceManager.h"
#include "DataManager/DataManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Common/Time.h"
#include "InputManager/InputManager.h"
#include "Utils/Assert.h"
#include "Utils/TimeCounter.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace JoyEngine
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	JoyEngine::JoyEngine(HINSTANCE instance, HWND windowHandle, uint32_t width, uint32_t height) :
		m_windowHandle(windowHandle),
		m_inputManager(new InputManager()),
		m_graphicsManager(new GraphicsManager(instance, windowHandle, width, height)),
		m_memoryManager(new MemoryManager()),
		m_dataManager(new DataManager()),
		m_descriptorSetManager(new DescriptorManager()),
		m_resourceManager(new ResourceManager()),
		m_sceneManager(new SceneManager()),
		m_renderManager(new RenderManager()),
		m_engineMaterials(new EngineMaterialProvider())
	{
		ASSERT(m_inputManager != nullptr);
		ASSERT(m_graphicsManager != nullptr);
		ASSERT(m_memoryManager != nullptr);
		ASSERT(m_dataManager != nullptr);
		ASSERT(m_descriptorSetManager != nullptr);
		ASSERT(m_resourceManager != nullptr);
		ASSERT(m_sceneManager != nullptr);
		ASSERT(m_renderManager != nullptr);
		ASSERT(m_engineMaterials != nullptr);

		{
			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			//ImGuiIO& io = ImGui::GetIO();
			//(void)io;
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

			ImGui::StyleColorsDark();
			ImGui_ImplWin32_Init(windowHandle);
		}
	}

	void JoyEngine::Init() const noexcept
	{
		Time::Init(m_deltaTimeHandler);

		// allocating memory for rendering and creating allocators
		m_memoryManager->Init();
		// allocating descriptors
		m_descriptorSetManager->Init();
		// creating internal engine materials
		m_engineMaterials->Init();
		// creating render resources
		m_renderManager->Init();
		// loading scene from disk
		m_sceneManager->Init();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		Logger::LogFormat("=========== Context initialized in %.3f seconds ===========\n", time);

		m_memoryManager->PrintStats();

		Logger::Log("==================================================================\n");
	}

	void JoyEngine::Start() const noexcept
	{
		m_renderManager->Start();
	}

	void JoyEngine::Update() const noexcept
	{
		Time::Update();

		m_sceneManager->Update();
		m_renderManager->Update();
	}

	void JoyEngine::Stop() noexcept
	{
		m_renderManager->Stop();

	}

	JoyEngine::~JoyEngine()
	{
		// will destroy managers in certain order
		m_inputManager = nullptr;
		m_sceneManager = nullptr; // unregister mesh renderers, remove descriptor set, pipelines, pipeline layouts
		m_engineMaterials = nullptr; //delete all internal engine resources
		m_renderManager = nullptr; //delete swapchain, synchronisation, framebuffers
		m_resourceManager = nullptr; //delete all scene render data (buffers, textures)
		m_descriptorSetManager = nullptr;
		m_dataManager = nullptr;
		m_memoryManager = nullptr; //free gpu memory
		m_graphicsManager = nullptr; //delete surface, device, instance

		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

#ifdef _DEBUG
		IDXGIDebug1* pDebug = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
		{
			pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
			pDebug->Release();
		}
#endif

		OutputDebugStringA("Context destroyed\n");
	}

	void JoyEngine::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

		m_inputManager->HandleWinMessage(hwnd, uMsg, wParam, lParam);
	}
}
