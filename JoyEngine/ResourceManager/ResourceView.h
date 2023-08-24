#ifndef HEAP_HANDLE_H
#define HEAP_HANDLE_H

#include <cstdint>

#include <d3d12.h>

namespace JoyEngine
{
	class ResourceView final
	{
	public:
		ResourceView() = delete;
		~ResourceView() = default;

		explicit ResourceView(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* resource);
		explicit ResourceView(const D3D12_SAMPLER_DESC&);
		explicit ResourceView(D3D12_CONSTANT_BUFFER_VIEW_DESC desc);
		explicit ResourceView(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource);
		explicit ResourceView(const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* resource);
		explicit ResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource, bool nonReadonlyTexture = false);

		[[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const noexcept { return m_type; }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const noexcept { return m_cpuHandle; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const noexcept { return m_gpuHandle; }

		[[nodiscard]] D3D12_DEPTH_STENCIL_VIEW_DESC GetDsvDesc() const noexcept { return m_description.dsvDesc; }
		[[nodiscard]] D3D12_SAMPLER_DESC GetSamplerDesc() const noexcept { return m_description.samplerDesc; }
		[[nodiscard]] D3D12_CONSTANT_BUFFER_VIEW_DESC GetConstantBufferDesc() const noexcept { return m_description.constantBufferDesc; }
		[[nodiscard]] D3D12_UNORDERED_ACCESS_VIEW_DESC GetUavDesc() const noexcept { return m_description.uavDesc; }
		[[nodiscard]] D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc() const noexcept { return m_description.rtvDesc; }
		[[nodiscard]] D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc() const noexcept { return m_description.srvDesc; }

		[[nodiscard]] uint32_t GetDescriptorIndex() const noexcept { return m_descriptorIndex; }


	private:
		union ViewDescription
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
			D3D12_SAMPLER_DESC samplerDesc;
			D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferDesc;
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		};

		ViewDescription m_description;

		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {0};
		D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {0};
		uint32_t m_descriptorIndex = 0;
	};
}
#endif // HEAP_HANDLE_H
