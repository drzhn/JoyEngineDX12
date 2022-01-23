#include "HeapHandle.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	HeapHandle::HeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12Resource* resource, DXGI_FORMAT format):
		m_type(type)
	{
		auto flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		switch (type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
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
				// Describe and create a SRV for the texture.
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;
				JoyContext::Graphics->GetDevice()->CreateShaderResourceView(
					resource,
					&srvDesc,
					m_handle);
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			{
				D3D12_SAMPLER_DESC samplerDesc = {};
				samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				samplerDesc.MinLOD = 0;
				samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
				samplerDesc.MipLODBias = 0.0f;
				samplerDesc.MaxAnisotropy = 1;
				samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				JoyContext::Graphics->GetDevice()->CreateSampler(
					&samplerDesc,
					m_handle);
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
				D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
				dsv.Format = DXGI_FORMAT_D32_FLOAT;
				dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsv.Texture2D.MipSlice = 0;
				dsv.Flags = D3D12_DSV_FLAG_NONE;
				JoyContext::Graphics->GetDevice()->CreateDepthStencilView(
					resource,
					&dsv,
					m_handle);
				break;
			}
		case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
			ASSERT(false);
		}
	}
}
