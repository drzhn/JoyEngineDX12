#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <vector>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <dxgidebug.h>

#include "Common/Singleton.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class GraphicsManager : public Singleton<GraphicsManager>
	{
	public:
		GraphicsManager() = delete;

		explicit GraphicsManager(HINSTANCE instance, HWND windowHandle, uint32_t width, uint32_t height);

		~GraphicsManager() = default;

		[[nodiscard]] HINSTANCE GetHINSTANCE() const noexcept { return m_windowInstance; }
		[[nodiscard]] HWND GetHWND() const noexcept { return m_windowHandle; }
		[[nodiscard]] IDXGIAdapter4* GetPhysicalDevice() const noexcept { return m_physicalDevice.Get(); }
		[[nodiscard]] ID3D12Device2* GetDevice() const noexcept { return m_logicalDevice.Get(); }
		[[nodiscard]] IDXGIFactory4* GetFactory() const noexcept { return m_dxgiFactory.Get(); }
		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }
		[[nodiscard]] bool GetTearingSupport() const noexcept { return m_allowTearing; }

		[[nodiscard]] D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion() const noexcept { return m_highestRootSignatureVersion; }

	private:
		uint32_t m_width;
		uint32_t m_height;

		HINSTANCE m_windowInstance;
		HWND m_windowHandle;

		ComPtr<IDXGIFactory5> m_dxgiFactory = nullptr;
		ComPtr<IDXGIAdapter4> m_physicalDevice = nullptr;
		ComPtr<ID3D12Device2> m_logicalDevice = nullptr;
#if defined(FULL_DEBUG)
		ComPtr<ID3D12Debug> m_debugController;

#endif
		uint32_t m_m4xMsaaQuality;
		bool m_allowTearing;
		D3D12_FEATURE_DATA_D3D12_OPTIONS m_featureSupport;
		D3D_ROOT_SIGNATURE_VERSION m_highestRootSignatureVersion;
	};
}

#endif //GRAPHICS_MANAGER_H
