#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "Common/Singleton.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class GraphicsManager : public Singleton<GraphicsManager>
	{
	public:
		GraphicsManager();

		~GraphicsManager() = default;

		[[nodiscard]] IDXGIAdapter4* GetPhysicalDevice() const noexcept { return m_physicalDevice.Get(); }
		[[nodiscard]] ID3D12Device5* GetDevice() const noexcept { return m_logicalDevice.Get(); }
		[[nodiscard]] IDXGIFactory5* GetFactory() const noexcept { return m_dxgiFactory.Get(); }
		[[nodiscard]] bool GetTearingSupport() const noexcept { return m_allowTearing; }

		[[nodiscard]] D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion() const noexcept { return m_highestRootSignatureVersion; }

	private:
		ComPtr<IDXGIFactory5> m_dxgiFactory = nullptr;
		ComPtr<IDXGIAdapter4> m_physicalDevice = nullptr;
		ComPtr<ID3D12Device5> m_logicalDevice = nullptr;

#if defined(GRAPHICS_DEBUG)
		ComPtr<ID3D12Debug> m_debugController;
#endif

		uint32_t m_m4xMsaaQuality;
		bool m_allowTearing;
		D3D12_FEATURE_DATA_D3D12_OPTIONS m_featureSupport;
		D3D_ROOT_SIGNATURE_VERSION m_highestRootSignatureVersion;
	};
}

#endif //GRAPHICS_MANAGER_H
