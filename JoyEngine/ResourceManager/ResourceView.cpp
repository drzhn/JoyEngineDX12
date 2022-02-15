#include "ResourceView.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	ResourceView::ResourceView(D3D12_DEPTH_STENCIL_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateDepthStencilView(
			resource,
			&desc,
			m_handle);
	}

	ResourceView::ResourceView(D3D12_SAMPLER_DESC desc):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateSampler(
			&desc,
			m_handle);
	}

	ResourceView::ResourceView(D3D12_CONSTANT_BUFFER_VIEW_DESC desc):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateConstantBufferView(
			&desc,
			m_handle);
	}

	ResourceView::ResourceView(D3D12_UNORDERED_ACCESS_VIEW_DESC desc, ID3D12Resource* resource):
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateUnorderedAccessView(
			resource,
			nullptr,
			&desc,
			m_handle);
	}

	ResourceView::ResourceView(D3D12_RENDER_TARGET_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateRenderTargetView(
			resource,
			&desc,
			m_handle);
	}

	ResourceView::ResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC desc, ID3D12Resource* resource) :
		m_type(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			m_type,
			1,
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		JoyContext::Graphics->GetDevice()->CreateShaderResourceView(
			resource,
			&desc,
			m_handle);
	}

}
