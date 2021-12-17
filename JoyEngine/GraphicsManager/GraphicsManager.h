#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <vector>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class GraphicsManager
	{
	public:
		GraphicsManager(HINSTANCE instance, HWND windowHandle);

		~GraphicsManager() = default;

		[[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept { return m_windowInstance; }
		[[nodiscard]] HWND GetHWND() const noexcept { return m_windowHandle; }
		[[nodiscard]] IDXGIAdapter4* GetPhysicalDevice() const noexcept { return m_physicalDevice.Get(); }
		[[nodiscard]] ID3D12Device2* GetDevice() const noexcept { return m_logicalDevice.Get(); }
		[[nodiscard]] IDXGIFactory4* GetFactory() const noexcept { return m_dxgiFactory.Get(); }

	private:
		HINSTANCE m_windowInstance;
		HWND m_windowHandle;

		ComPtr<IDXGIFactory4> m_dxgiFactory = nullptr;
		ComPtr<IDXGIAdapter4> m_physicalDevice = nullptr;
		ComPtr<ID3D12Device2> m_logicalDevice = nullptr;

		//Allocator* m_allocator;
		//VkInstance m_vkInstance;
		//VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		//VkDevice m_logicalDevice;
		//VkDebugUtilsMessengerEXT m_debugMessenger;
		//VkSurfaceKHR m_surface;

		//std::unique_ptr<QueueFamilyIndices> m_queueFamilyIndices;
		//VkQueue m_graphicsQueue;
		//VkQueue m_presentQueue;
		//VkQueue m_transferQueue;

		//VkCommandPool m_commandPool;
	};
}

#endif //GRAPHICS_MANAGER_H
