#include "Texture.h"

#include <utility>

#include "JoyContext.h"
#include "DataManager/DataManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	Texture::Texture(GUID guid) :
		Resource(guid),
		m_format(DXGI_FORMAT_R8G8B8A8_UNORM),
		m_usageFlags(D3D12_RESOURCE_STATE_COPY_DEST),
		m_propertiesFlags(D3D12_HEAP_TYPE_DEFAULT)
	{
		m_textureStream = JoyContext::Data->GetFileStream(guid, true);
		uint32_t width, height;

		m_textureStream.seekg(0);
		m_textureStream.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
		m_textureStream.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

		m_width = width;
		m_height = height;

		CreateImage();
		CreateImageView();
		CreateImageSampler();
		JoyContext::Memory->LoadDataToImage(m_textureStream, sizeof(uint32_t) + sizeof(uint32_t), m_width, m_height,
		                                    m_texture);
	}

	Texture::Texture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties):
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_propertiesFlags(properties)
	{
		CreateImage();
		CreateImageView();
		CreateImageSampler();
	}

	Texture::Texture(
		ComPtr<ID3D12Resource> externalResource,
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties) :
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_propertiesFlags(properties),
		m_texture(std::move(externalResource))
	{
		CreateImageView();
		CreateImageSampler();
	}

	void Texture::CreateImage()
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = {1.0f, 0};

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = m_format;
		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.Flags = m_format == DXGI_FORMAT_D32_FLOAT ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateCommittedResource(
				&m_propertiesFlags,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				m_usageFlags,
				m_format == DXGI_FORMAT_D32_FLOAT ? &optimizedClearValue : nullptr,
				IID_PPV_ARGS(&m_texture))
		);
	}

	void Texture::CreateImageView()
	{
		auto heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		auto flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		if ((m_usageFlags & D3D12_RESOURCE_STATE_RENDER_TARGET) != 0)
		{
			heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		}
		if ((m_usageFlags & D3D12_RESOURCE_STATE_DEPTH_WRITE) != 0)
		{
			heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		}

		// create descriptor heap for shader resource view
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			heapType,
			1,
			flags,
			0
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(&m_resourceViewDescriptorHeap)));

		m_textureImageView = m_resourceViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		if ((m_usageFlags & D3D12_RESOURCE_STATE_RENDER_TARGET) != 0)
		{
			JoyContext::Graphics->GetDevice()->CreateRenderTargetView(
				m_texture.Get(),
				nullptr,
				m_textureImageView);
		}
		else if ((m_usageFlags & D3D12_RESOURCE_STATE_DEPTH_WRITE) != 0)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
			dsv.Format = DXGI_FORMAT_D32_FLOAT;
			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = D3D12_DSV_FLAG_NONE;

			JoyContext::Graphics->GetDevice()->CreateDepthStencilView(m_texture.Get(), &dsv,
				m_resourceViewDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		}
		else
		{
			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = m_format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			JoyContext::Graphics->GetDevice()->CreateShaderResourceView(
				m_texture.Get(),
				&srvDesc,
				m_textureImageView);
		}
	}

	void Texture::CreateImageSampler()
	{
		// create sampler descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
		descHeapSampler.NumDescriptors = 1;
		descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateDescriptorHeap(
			&descHeapSampler,
			IID_PPV_ARGS(&m_samplerDescriptorHeap)));

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
		m_textureSampler = m_samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		JoyContext::Graphics->GetDevice()->CreateSampler(
			&samplerDesc,
			m_textureSampler);
	}
}
