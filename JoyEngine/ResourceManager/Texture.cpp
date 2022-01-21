﻿#include "Texture.h"

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
		m_memoryPropertiesFlags(D3D12_HEAP_TYPE_DEFAULT)
	{
		auto textureStream = JoyContext::Data->GetFileStream(guid, true);
		uint32_t width, height;

		textureStream.seekg(0);
		textureStream.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
		textureStream.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));

		m_width = width;
		m_height = height;

		CreateImage();
		CreateImageView();
		CreateImageSampler();
		JoyContext::Memory->LoadDataToImage(
			textureStream,
			sizeof(uint32_t) + sizeof(uint32_t),
			m_width,
			m_height,
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
		m_memoryPropertiesFlags(properties)
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
		m_memoryPropertiesFlags(properties),
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
				&m_memoryPropertiesFlags,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				m_usageFlags,
				m_format == DXGI_FORMAT_D32_FLOAT ? &optimizedClearValue : nullptr,
				IID_PPV_ARGS(&m_texture))
		);
	}

	void Texture::CreateImageView()
	{
		if ((m_usageFlags & D3D12_RESOURCE_STATE_RENDER_TARGET) != 0)
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, this);
		}
		else if ((m_usageFlags & D3D12_RESOURCE_STATE_DEPTH_WRITE) != 0)
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, this);
		}
		else
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, this);
		}
	}

	void Texture::CreateImageSampler()
	{
		m_samplerView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, this);
	}
}
