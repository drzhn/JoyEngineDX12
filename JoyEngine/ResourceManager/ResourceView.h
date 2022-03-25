#ifndef HEAP_HANDLE_H
#define HEAP_HANDLE_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <fstream>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class ResourceView
	{
	public:
		ResourceView() = default;

		explicit ResourceView(D3D12_DEPTH_STENCIL_VIEW_DESC desc, ID3D12Resource* resource);
		explicit ResourceView(D3D12_SAMPLER_DESC);
		explicit ResourceView(D3D12_CONSTANT_BUFFER_VIEW_DESC desc);
		explicit ResourceView(D3D12_UNORDERED_ACCESS_VIEW_DESC desc, ID3D12Resource* resource);
		explicit ResourceView(D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource* resource);
		explicit ResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC desc, ID3D12Resource* resource);

		[[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const noexcept { return m_type; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const noexcept { return m_cpuHandle; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const noexcept { return m_gpuHandle; }

	private:
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {0};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {0};
		uint32_t m_descriptorIndex = 0;
	};
}
#endif // HEAP_HANDLE_H
