#include "Texture.h"

#include <utility>

#include "JoyContext.h"
#include "DataManager/DataManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/Assert.h"
#include "DDS.h"
#include "DDSTextureLoader.h"
#include "JoyEngine.h"

using namespace DirectX;

namespace JoyEngine
{
	std::unique_ptr<ResourceView> Texture::m_textureSampler = nullptr;
	std::unique_ptr<ResourceView> Texture::m_depthPCFSampler = nullptr;
	std::unique_ptr<ResourceView> Texture::m_depthSampler = nullptr;
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

		D3D12_SAMPLER_DESC depthSamplerDesc = {};
		depthSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		depthSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		depthSamplerDesc.BorderColor[0] = 1.0f;
		depthSamplerDesc.BorderColor[1] = 1.0f;
		depthSamplerDesc.BorderColor[2] = 1.0f;
		depthSamplerDesc.BorderColor[3] = 1.0f;
		depthSamplerDesc.MinLOD = 0;
		depthSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		depthSamplerDesc.MipLODBias = 0.0f;
		depthSamplerDesc.MaxAnisotropy = 1;
		depthSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_depthSampler = std::make_unique<ResourceView>(depthSamplerDesc);

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

	ResourceView* Texture::GetDepthSampler()
	{
		ASSERT(m_depthSampler != nullptr);
		return m_depthSampler.get();
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
		bool hasRawData = JoyContext::Data->HasRawData(guid);
		auto textureStream = JoyContext::Data->GetFileStream(guid, hasRawData);

		textureStream.seekg(0);
		uint32_t ddsMagicNumber = 0;
		textureStream.read(reinterpret_cast<char*>(&ddsMagicNumber), sizeof(uint32_t));
		if (ddsMagicNumber == DDS_MAGIC)
		{
			DDS_HEADER header = {};
			textureStream.read(reinterpret_cast<char*>(&header), sizeof(DDS_HEADER));
			m_width = header.width;
			m_height = header.height;
			m_format = DXGI_FORMAT_BC1_UNORM;
			//CreateImage(false, false, false, 1, 1);
			ID3D12Resource* resource = nullptr; // = m_texture.Get();
			uint32_t pitch = header.pitchOrLinearSize;
			// yes, i know i unload data two times.
			// TODO make special d3d12heap for intermediate data and allocate data there
			std::vector<char> data = JoyContext::Data->GetData(guid, false, 0);
			std::vector<D3D12_SUBRESOURCE_DATA> subresource;
			LoadDDSTextureFromMemory(
				JoyContext::Graphics->GetDevice(),
				reinterpret_cast<uint8_t*>(data.data()),
				data.size(),
				&resource,
				subresource
			);
			m_texture = resource;
			CreateImageView(false, false, false, 1);
			JoyContext::Memory->LoadDataToImage(
				textureStream,
				sizeof(uint32_t) + sizeof(DDS_HEADER),
				subresource[0].RowPitch,
				subresource[0].SlicePitch,
				m_width,
				m_height,
				this,
				1);
		}
		else
		{
			InitTextureFromFile(textureStream);
		}
	}

	Texture::Texture(
		GUID guid,
		const std::string& file) :
		Resource(guid),
		m_usageFlags(D3D12_RESOURCE_STATE_COPY_DEST),
		m_memoryPropertiesFlags(D3D12_HEAP_TYPE_DEFAULT)
	{
		const bool hasRawData = JoyContext::Data->HasRawData(file);
		auto textureStream = JoyContext::Data->GetFileStream(file, hasRawData);

		InitTextureFromFile(textureStream);
	}

	void Texture::InitTextureFromFile(std::ifstream& textureStream) // TODO make this DDS compatible
	{
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
		CreateImageView(false, false, false, 1);
		JoyContext::Memory->LoadDataToImage(
			textureStream,
			sizeof(uint32_t) + sizeof(uint32_t),
			m_width * 4,
			m_width * m_height * 4,
			m_width,
			m_height,
			this,
			1);
	}

	Texture::Texture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties,
		bool allowRenderTarget,
		bool isDepthTarget,
		bool allowUnorderedAccess,
		uint32_t arraySize):
		m_width(width),
		m_height(height),
		m_format(format),
		m_usageFlags(usage),
		m_memoryPropertiesFlags(properties)
	{
		CreateImage(allowRenderTarget, isDepthTarget, allowUnorderedAccess, arraySize);
		CreateImageView(allowRenderTarget, isDepthTarget, allowUnorderedAccess, arraySize);
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
		CreateImageView(true, false, false, 1); // I use this only for creating texture from system back buffer
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
		else if (isDepthTarget)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			optimizedClearValue.DepthStencil = {1.0f, 0};
		}
		else if (allowRenderTarget)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
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

		m_texture = JoyContext::Memory->CreateResource(
			m_memoryPropertiesFlags.Type,
			&textureDesc,
			m_usageFlags,
			isDepthTarget ? &optimizedClearValue : nullptr);
	}

	void Texture::CreateImageView(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize)
	{
		ASSERT(!(allowRenderTarget && isDepthTarget ));
		ASSERT(!(allowRenderTarget && allowUnorderedAccess));
		ASSERT(!(isDepthTarget && allowUnorderedAccess));
		ASSERT(arraySize >= 1);

		if (allowRenderTarget)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc;
			desc.Format = m_format;
			if (arraySize == 1)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = 0;
				desc.Texture2D.PlaneSlice = 0;
				m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get());
			}
			else if (arraySize == 6)
			{
				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				for (uint32_t i = 0; i < 6; i++)
				{
					desc.Texture2DArray.ArraySize = 1; // view from single element
					desc.Texture2DArray.FirstArraySlice = i;
					desc.Texture2DArray.MipSlice = 0;
					desc.Texture2DArray.PlaneSlice = 0;
					m_resourceViewArray[i] = std::make_unique<ResourceView>(desc, m_texture.Get());
				}

				desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.ArraySize = arraySize;
				desc.Texture2DArray.FirstArraySlice = 0;
				desc.Texture2DArray.MipSlice = 0;
				desc.Texture2DArray.PlaneSlice = 0;
				m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get());
			}
			else
			{
				ASSERT(false); // can we have other rtv arrays?
			}
		}
		else if (isDepthTarget)
		{
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
		else if (allowUnorderedAccess)
		{
			ASSERT(arraySize == 1);
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = m_format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; // TODO make for arraySize > 1
			uavDesc.Texture2D = {
				0,
				0
			};
			m_resourceView = std::make_unique<ResourceView>(uavDesc, m_texture.Get());
		}
		else
		{
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
		D3D12_HEAP_TYPE properties,
		uint32_t arraySize):
		Texture(width,
		        height,
		        format,
		        usage,
		        properties,
		        true,
		        false,
		        false,
		        arraySize)
	{
		ASSERT(arraySize > 0);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = this->GetFormat();
		if (arraySize == 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D = {
				0,
				1,
				0,
				0
			};
		}
		else
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MipLevels = 1;
			desc.TextureCube.MostDetailedMip = 0;
			desc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
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
		        false,
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

	UAVTexture::UAVTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE properties,
		uint32_t arraySize):
		Texture(width,
		        height,
		        format,
		        usage,
		        properties,
		        false,
		        false,
		        true,
		        arraySize)
	{
		ASSERT(arraySize > 0);

		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = this->GetFormat();
		if (arraySize == 1)
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D = {
				0,
				1,
				0,
				0
			};
		}
		else
		{
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			desc.TextureCube.MipLevels = 1;
			desc.TextureCube.MostDetailedMip = 0;
			desc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		m_inputAttachmentView = std::make_unique<ResourceView>(
			desc,
			this->GetImage().Get()
		);
	}
}
