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
	std::unique_ptr<ResourceView> Texture::m_pointSampler = nullptr;

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
		m_depthPCFSampler = std::make_unique<ResourceView>(depthPCFSamplerDesc);

		D3D12_SAMPLER_DESC pointSamplerDesc = {};
		pointSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		pointSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointSamplerDesc.BorderColor[0] = 1.0f;
		pointSamplerDesc.BorderColor[1] = 1.0f;
		pointSamplerDesc.BorderColor[2] = 1.0f;
		pointSamplerDesc.BorderColor[3] = 1.0f;
		pointSamplerDesc.MinLOD = 0;
		pointSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		pointSamplerDesc.MipLODBias = 0.0f;
		pointSamplerDesc.MaxAnisotropy = 1;
		pointSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		m_pointSampler = std::make_unique<ResourceView>(pointSamplerDesc);
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

	ResourceView* Texture::GetPointSampler()
	{
		ASSERT(m_pointSampler != nullptr);
		return m_pointSampler.get();
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

		CreateImage(false, false, true, 1, 5);
		CreateImageView(false, false, 1);
		JoyContext::Memory->LoadDataToImage(
			textureStream,
			sizeof(uint32_t) + sizeof(uint32_t),
			m_width,
			m_height,
			this);
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
		CreateImage(allowRenderTarget, isDepthTarget, false, arraySize);
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

	void Texture::CreateImage(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize, uint32_t mipLevels)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
		ASSERT(!(allowRenderTarget && isDepthTarget));

		if (allowUnorderedAccess)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

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
		textureDesc.MipLevels = mipLevels;
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
			D3D12_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = m_format;
			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
			m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get());
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
			D3D12_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = m_format;
			desc.ViewDimension = arraySize == 1 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.Texture2D = {
			0,
			1,
			0,
			0
			};
			m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get()); // TODO cube or 2dArray?
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
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = this->GetFormat();
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // arraySize == 1 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.Texture2D = {
			0,
			1,
			0,
			0
		};
		m_inputAttachmentView = std::make_unique<ResourceView>(
			desc,
			this->GetImage().Get()
		);
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
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = DXGI_FORMAT_R32_FLOAT;
		desc.ViewDimension = arraySize == 1 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE;
		desc.Texture2D = {
		0,
		1,
		0,
		0
		};
		m_inputAttachmentView = std::make_unique<ResourceView>(
			desc,
			this->GetImage().Get());
	}
}
