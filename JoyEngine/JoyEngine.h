#ifndef JOY_ENGINE_H
#define JOY_ENGINE_H

#include <functional>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace JoyEngine
{
	class InputManager;

	class ThreadManager;

	class GraphicsManager;

	class MemoryManager;

	class DataManager;

	class DescriptorManager;

	class ResourceManager;

	class WorldManager;

	class RenderManager;

	class EngineDataProvider;

	class IWindowHandler
	{
	public:
		virtual void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

		void SetDeltaTimeHandler(const std::function<void(float)> handler)
		{
			m_deltaTimeHandler = handler;
		}

		virtual void GetScreenSize(uint32_t& width, uint32_t& height) = 0;

	protected:
		std::function<void(float)> m_deltaTimeHandler;
	};

	class JoyEngine final : public IWindowHandler
	{
	public:
		JoyEngine(HWND gameWindowHandle);

		void Init() const noexcept;

		void Start() const noexcept;

		void UpdateTask() const noexcept;

		void Stop() const noexcept;

		~JoyEngine();

		void HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		void GetScreenSize(uint32_t& width, uint32_t& height) override;

	private:
		HWND m_windowHandle;

		std::unique_ptr<InputManager> m_inputManager;
		std::unique_ptr<ThreadManager> m_threadManager;
		std::unique_ptr<GraphicsManager> m_graphicsManager;
		std::unique_ptr<MemoryManager> m_memoryManager;
		std::unique_ptr<DataManager> m_dataManager;
		std::unique_ptr<DescriptorManager> m_descriptorSetManager;
		std::unique_ptr<ResourceManager> m_resourceManager;
		std::unique_ptr<RenderManager> m_renderManager;
		std::unique_ptr<EngineDataProvider> m_engineData;
		std::unique_ptr<WorldManager> m_worldManager;
	};
}


#endif//JOY_ENGINE_H
