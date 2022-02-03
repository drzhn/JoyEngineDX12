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
		m_usageFlags(D3D12_RESOURCE_STATE_COPY_DEST),
		m_memoryPropertiesFlags(D3D12_HEAP_TYPE_DEFAULT)
	{
		auto textureStream = JoyContext::Data->GetFileStream(guid, true);
		uint32_t width, height;
		TextureType type;

		textureStream.seekg(0);
		textureStream.read(reinterpret_cast<char*>(&width), sizeof(uint32_t));
		textureStream.read(reinterpret_cast<char*>(&height), sizeof(uint32_t));
		textureStream.read(reinterpret_cast<char*>(&type), sizeof(uint32_t));

		m_width = width;
		m_height = height;
		switch (type)
		{
		case RGBA_UNORM:
			m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case RGB_FLOAT:
			m_format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		default:
			ASSERT(false);
		}

		CreateImage(false);
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
		D3D12_HEAP_TYPE properties,
		bool allowRenderTarget):
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_memoryPropertiesFlags(properties)
	{
		CreateImage(allowRenderTarget);
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

	void Texture::CreateImage(bool allowRenderTarget)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = {1.0f, 0};

		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		if (m_format == DXGI_FORMAT_D32_FLOAT)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}
		else
		{
			if (allowRenderTarget)
			{
				flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
		}

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = m_format;
		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.Flags = flags;
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
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_texture.Get(), m_format);
		}
		else if ((m_usageFlags & D3D12_RESOURCE_STATE_DEPTH_WRITE) != 0)
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_texture.Get(), m_format);
		}
		else
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_texture.Get(), m_format);
		}
	}

	void Texture::CreateImageSampler()
	{
		m_samplerView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, m_texture.Get(), m_format);
	}

	RenderTexture::RenderTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties):
		Texture(width,
		        height,
		        format,
		        usage,
		        properties,
		        true)
	{
		m_inputAttachmentView = std::make_unique<HeapHandle>(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			this->GetImage().Get(),
			this->GetFormat());
	}
}
