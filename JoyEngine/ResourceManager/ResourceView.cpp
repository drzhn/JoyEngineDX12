#include "ResourceView.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	ResourceView::ResourceView(D3D12_DEPTH_STENCIL_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateDepthStencilView(
			resource,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_SAMPLER_DESC desc):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateSampler(
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_CONSTANT_BUFFER_VIEW_DESC desc):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateConstantBufferView(
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_UNORDERED_ACCESS_VIEW_DESC desc, ID3D12Resource* resource):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateUnorderedAccessView(
			resource,
			nullptr,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateRenderTargetView(
			resource,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		JoyContext::Descriptors->AllocateDescriptor(m_type, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		JoyContext::Graphics->GetDevice()->CreateShaderResourceView(
			resource,
			&desc,
			m_cpuHandle);
	}

}
