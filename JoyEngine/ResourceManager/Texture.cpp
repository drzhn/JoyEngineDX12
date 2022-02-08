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
	std::unique_ptr<HeapHandle> Texture::m_textureSampler = nullptr;
	std::unique_ptr<HeapHandle> Texture::m_depthPCFSampler = nullptr;

	void Texture::InitSamplers()
	{
		D3D12_SAMPLER_DESC textureSamplerDesc = {};
		textureSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		textureSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		textureSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		textureSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		textureSamplerDesc.MinLOD = 0;
		textureSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		textureSamplerDesc.MipLODBias = 0.0f;
		textureSamplerDesc.MaxAnisotropy = 1;
		textureSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_textureSampler = std::make_unique<HeapHandle>(textureSamplerDesc);

		D3D12_SAMPLER_DESC depthPCFSamplerDesc = {};
		depthPCFSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		depthPCFSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthPCFSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthPCFSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthPCFSamplerDesc.BorderColor[0] = 1.0f;
		depthPCFSamplerDesc.BorderColor[1] = 1.0f;
		depthPCFSamplerDesc.BorderColor[2] = 1.0f;
		depthPCFSamplerDesc.BorderColor[3] = 1.0f;
		depthPCFSamplerDesc.MinLOD = 0;
		depthPCFSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		depthPCFSamplerDesc.MipLODBias = 0.0f;
		depthPCFSamplerDesc.MaxAnisotropy = 1;
		depthPCFSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
		m_depthPCFSampler = std::make_unique<HeapHandle>(depthPCFSamplerDesc);
	}

	HeapHandle* Texture::GetTextureSampler()
	{
		ASSERT(m_textureSampler != nullptr);
		return m_textureSampler.get();
	}

	HeapHandle* Texture::GetDepthPCFSampler()
	{
		ASSERT(m_depthPCFSampler != nullptr);
		return m_depthPCFSampler.get();
	}

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

		CreateImage(false, false);
		CreateImageView(false, false);
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
		bool allowRenderTarget,
		bool isDepthTarget):
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_memoryPropertiesFlags(properties)
	{
		CreateImage(allowRenderTarget, isDepthTarget);
		CreateImageView(allowRenderTarget, isDepthTarget);
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
		CreateImageView(true, false); // I use this only for creating texture from system back buffer
	}

	void Texture::CreateImage(bool allowRenderTarget, bool isDepthTarget)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		ASSERT(!(allowRenderTarget && isDepthTarget));

		if (isDepthTarget)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			optimizedClearValue.DepthStencil = {1.0f, 0};
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
				isDepthTarget ? &optimizedClearValue : nullptr,
				IID_PPV_ARGS(&m_texture))
		);
	}

	void Texture::CreateImageView(bool allowRenderTarget, bool isDepthTarget)
	{
		ASSERT(!(allowRenderTarget && isDepthTarget));
		if (allowRenderTarget)
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_texture.Get(), m_format);
		}
		else if (isDepthTarget)
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_texture.Get(), m_format);
		}
		else
		{
			m_resourceView = std::make_unique<HeapHandle>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_texture.Get(), m_format);
		}
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

	DepthTexture::DepthTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties) :
		Texture(width,
		        height,
		        format,
		        usage,
		        properties,
		        false,
		        true)
	{
		m_inputAttachmentView = std::make_unique<HeapHandle>(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			this->GetImage().Get(),
			DXGI_FORMAT_R32_FLOAT);
	}
}
