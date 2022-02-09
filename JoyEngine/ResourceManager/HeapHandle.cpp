#include "HeapHandle.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	HeapHandle::HeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12Resource* resource, DXGI_FORMAT format, D3D12_SRV_DIMENSION dimension):
		m_type(type)
	{
		auto flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			ASSERT(false);
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
			ASSERT(false);
		}

		// create descriptor heap for shader resource view
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			type,
			1,
			flags,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)));

		m_handle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			{
				if (dimension == D3D12_SRV_DIMENSION_BUFFER)
				{
					ASSERT(false);
				}
				else
				{
					// Describe and create a SRV for the texture.
					D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					srvDesc.Format = format;
					srvDesc.ViewDimension = dimension;
					srvDesc.Texture2D.MipLevels = 1;
					JoyContext::Graphics->GetDevice()->CreateShaderResourceView(
						resource,
						&srvDesc,
						m_handle);
				}
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			{
				ASSERT(false);
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			{
				JoyContext::Graphics->GetDevice()->CreateRenderTargetView(
					resource,
					nullptr,
					m_handle);
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			{
				//D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
				//dsv.Format = DXGI_FORMAT_D32_FLOAT;
				//dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				//dsv.Texture2D.MipSlice = 0;
				//dsv.Flags = D3D12_DSV_FLAG_NONE;
				//JoyContext::Graphics->GetDevice()->CreateDepthStencilView(
				//	resource,
				//	&dsv,
				//	m_handle);
				ASSERT(false);
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
			ASSERT(false);
		}
	}

	HeapHandle::HeapHandle(D3D12_DEPTH_STENCIL_VIEW_DESC desc, ID3D12Resource* resource) :
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

	HeapHandle::HeapHandle(D3D12_SAMPLER_DESC desc):
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

	HeapHandle::HeapHandle(D3D12_CONSTANT_BUFFER_VIEW_DESC desc):
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
}
