#include "ResourceView.h"


#include "GraphicsManager/GraphicsManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	ResourceView::ResourceView(D3D12_DEPTH_STENCIL_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
	{
		m_description.dsvDesc = desc;
		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::DSV, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateDepthStencilView(
			resource,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_SAMPLER_DESC desc) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		m_description.samplerDesc = desc;

		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::SAMPLER, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateSampler(
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_CONSTANT_BUFFER_VIEW_DESC desc) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		m_description.constantBufferDesc = desc;

		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::CBV_UAV, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateConstantBufferView(
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_UNORDERED_ACCESS_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		m_description.uavDesc = desc;

		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::CBV_UAV, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateUnorderedAccessView(
			resource,
			nullptr,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		m_description.rtvDesc = desc;

		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::RTV, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateRenderTargetView(
			resource,
			&desc,
			m_cpuHandle);
	}

	ResourceView::ResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		m_description.srvDesc = desc;

		DescriptorManager::Get()->AllocateDescriptor(DescriptorHeapType::SRV, m_descriptorIndex, m_cpuHandle, m_gpuHandle);

		GraphicsManager::Get()->GetDevice()->CreateShaderResourceView(
			resource,
			&desc,
			m_cpuHandle);
	}

}
