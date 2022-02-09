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
	std::unique_ptr<ResourceView> Texture::m_textureSampler = nullptr;
	std::unique_ptr<ResourceView> Texture::m_depthPCFSampler = nullptr;

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
		m_textureSampler = std::make_unique<ResourceView>(textureSamplerDesc);

		D3D12_SAMPLER_DESC depthPCFSamplerDesc = {};
		depthPCFSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
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
		m_depthPCFSampler = std::make_unique<ResourceView>(depthPCFSamplerDesc);
	}

	ResourceView* Texture::GetTextureSampler()
	{
		ASSERT(m_textureSampler != nullptr);
		return m_textureSampler.get();
	}

	ResourceView* Texture::GetDepthPCFSampler()
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

		CreateImage(false, false, 1);
		CreateImageView(false, false, 1);
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
		bool isDepthTarget,
		uint32_t arraySize):
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_memoryPropertiesFlags(properties)
	{
		CreateImage(allowRenderTarget, isDepthTarget, arraySize);
		CreateImageView(allowRenderTarget, isDepthTarget, arraySize);
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
		CreateImageView(true, false, 1); // I use this only for creating texture from system back buffer
	}

	void Texture::CreateImage(bool allowRenderTarget, bool isDepthTarget, uint32_t arraySize)
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
		textureDesc.DepthOrArraySize = arraySize;
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

	void Texture::CreateImageView(bool allowRenderTarget, bool isDepthTarget, uint32_t arraySize)
	{
		ASSERT(!(allowRenderTarget && isDepthTarget));
		if (allowRenderTarget)
		{
			ASSERT(arraySize == 1);
			m_resourceView = std::make_unique<ResourceView>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_texture.Get(), m_format);
		}
		else if (isDepthTarget)
		{
			ASSERT(arraySize >= 1);

			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
			depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilViewDesc.ViewDimension = arraySize == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			if (arraySize == 1)
			{
				depthStencilViewDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				depthStencilViewDesc.Texture2DArray.ArraySize = arraySize;
			}
			depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

			m_resourceView = std::make_unique<ResourceView>(depthStencilViewDesc, m_texture.Get());
		}
		else
		{
			ASSERT(arraySize >= 1);

			m_resourceView = std::make_unique<ResourceView>(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				m_texture.Get(),
				m_format,
				arraySize == 1 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE); // TODO cube or 2dArray?
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
		m_inputAttachmentView = std::make_unique<ResourceView>(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			this->GetImage().Get(),
			this->GetFormat());
	}


	DepthTexture::DepthTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties,
		uint32_t arraySize) :
		Texture(width,
		        height,
		        format,
		        usage,
		        properties,
		        false,
		        true,
		        arraySize)
	{
		m_inputAttachmentView = std::make_unique<ResourceView>(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			this->GetImage().Get(),
			DXGI_FORMAT_R32_FLOAT);
	}
}
