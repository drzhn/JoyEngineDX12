#include "Texture.h"

#include <utility>


#include "DataManager/DataManager.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/Assert.h"
#include "DDS.h"
#include "JoyAssetHeaders.h"
#include "JoyEngine.h"

using namespace DirectX;

namespace JoyEngine
{
	std::unique_ptr<ResourceView> EngineSamplersProvider::m_linearWrapSampler = nullptr;
	std::unique_ptr<ResourceView> EngineSamplersProvider::m_linearClampSampler = nullptr;
	std::unique_ptr<ResourceView> EngineSamplersProvider::m_depthPCFSampler = nullptr;
	std::unique_ptr<ResourceView> EngineSamplersProvider::m_linearBorderWhiteSampler = nullptr;
	std::unique_ptr<ResourceView> EngineSamplersProvider::m_pointClampSampler = nullptr;

	void EngineSamplersProvider::InitSamplers()
	{
		D3D12_SAMPLER_DESC linearWrapSamplerDesc = {};
		linearWrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearWrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		linearWrapSamplerDesc.MinLOD = 0;
		linearWrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		linearWrapSamplerDesc.MipLODBias = 0.0f;
		linearWrapSamplerDesc.MaxAnisotropy = 1;
		linearWrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_linearWrapSampler = std::make_unique<ResourceView>(linearWrapSamplerDesc);

		D3D12_SAMPLER_DESC linearClampSamplerDesc = {};
		linearClampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearClampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		linearClampSamplerDesc.MinLOD = 0;
		linearClampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		linearClampSamplerDesc.MipLODBias = 0.0f;
		linearClampSamplerDesc.MaxAnisotropy = 1;
		linearClampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		m_linearClampSampler = std::make_unique<ResourceView>(linearClampSamplerDesc);

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

		D3D12_SAMPLER_DESC linearBorderWhiteSamplerDesc = {};
		linearBorderWhiteSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		linearBorderWhiteSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		linearBorderWhiteSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		linearBorderWhiteSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		linearBorderWhiteSamplerDesc.BorderColor[0] = 1.0f;
		linearBorderWhiteSamplerDesc.BorderColor[1] = 1.0f;
		linearBorderWhiteSamplerDesc.BorderColor[2] = 1.0f;
		linearBorderWhiteSamplerDesc.BorderColor[3] = 1.0f;
		linearBorderWhiteSamplerDesc.MinLOD = 0;
		linearBorderWhiteSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		linearBorderWhiteSamplerDesc.MipLODBias = 0.0f;
		linearBorderWhiteSamplerDesc.MaxAnisotropy = 1;
		linearBorderWhiteSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		m_linearBorderWhiteSampler = std::make_unique<ResourceView>(linearBorderWhiteSamplerDesc);

		D3D12_SAMPLER_DESC pointClampDesc = {};
		pointClampDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		pointClampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		pointClampDesc.BorderColor[0] = 1.0f;
		pointClampDesc.BorderColor[1] = 1.0f;
		pointClampDesc.BorderColor[2] = 1.0f;
		pointClampDesc.BorderColor[3] = 1.0f;
		pointClampDesc.MinLOD = 0;
		pointClampDesc.MaxLOD = D3D12_FLOAT32_MAX;
		pointClampDesc.MipLODBias = 0.0f;
		pointClampDesc.MaxAnisotropy = 1;
		pointClampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		m_pointClampSampler = std::make_unique<ResourceView>(pointClampDesc);
	}

	ResourceView* EngineSamplersProvider::GetLinearWrapSampler()
	{
		ASSERT(m_linearWrapSampler != nullptr);
		return m_linearWrapSampler.get();
	}

	ResourceView* EngineSamplersProvider::GetLinearClampSampler()
	{
		ASSERT(m_linearClampSampler != nullptr);
		return m_linearClampSampler.get();
	}

	ResourceView* EngineSamplersProvider::GetDepthPCFSampler()
	{
		ASSERT(m_depthPCFSampler != nullptr);
		return m_depthPCFSampler.get();
	}

	ResourceView* EngineSamplersProvider::GetLinearBorderWhiteSampler()
	{
		ASSERT(m_linearBorderWhiteSampler != nullptr);
		return m_linearBorderWhiteSampler.get();
	}

	ResourceView* EngineSamplersProvider::GetPointClampSampler()
	{
		ASSERT(m_pointClampSampler != nullptr);
		return m_pointClampSampler.get();
	}


	class TextureUtils
	{
	public:
		static std::unique_ptr<ResourceView> CreateSRV(DXGI_FORMAT format, uint32_t mipLevels, ID3D12Resource* resource,
		                                               bool nonReadonly = true)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = format;
			desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D = {
				0,
				mipLevels,
				0,
				0
			};
			return std::move(std::make_unique<ResourceView>(desc, resource, nonReadonly));
		}

		static std::unique_ptr<ResourceView> CreateRTV(DXGI_FORMAT format, ID3D12Resource* resource)
		{
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = format;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			return std::move(std::make_unique<ResourceView>(rtvDesc, resource));
		}

		static std::unique_ptr<ResourceView> CreateUAV(DXGI_FORMAT format, ID3D12Resource* resource)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			uavDesc.Format = format;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D = {
				0,
				0
			};

			return std::move(std::make_unique<ResourceView>(uavDesc, resource));
		}

		static std::unique_ptr<ResourceView> CreateDSV(DXGI_FORMAT format, uint32_t arraySize, ID3D12Resource* resource)
		{
			D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
			depthStencilViewDesc.Format = format;
			depthStencilViewDesc.ViewDimension = arraySize == 1
				                                     ? D3D12_DSV_DIMENSION_TEXTURE2D
				                                     : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			if (arraySize == 1)
			{
				depthStencilViewDesc.Texture2D.MipSlice = 0;
			}
			else
			{
				depthStencilViewDesc.Texture2DArray.ArraySize = arraySize;
			}
			depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

			return std::move(std::make_unique<ResourceView>(depthStencilViewDesc, resource));
		}
	};

	void AbstractTextureResource::CreateImageResource(
		bool allowRenderTarget,
		bool isDepthTarget,
		bool allowUnorderedAccess, uint32_t arraySize, uint32_t mipLevels)
	{
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

		ASSERT(!(isDepthTarget && allowRenderTarget));
		ASSERT(!(isDepthTarget && allowUnorderedAccess));
		ASSERT(arraySize >= 1);


		if (isDepthTarget)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			optimizedClearValue.DepthStencil = {1.0f, 0};
		}
		else
		{
			if (allowUnorderedAccess)
			{
				flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			if (allowRenderTarget)
			{
				flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				optimizedClearValue.Format = m_format;
				optimizedClearValue.Color[0] = 0.f;
				optimizedClearValue.Color[1] = 0.f;
				optimizedClearValue.Color[2] = 0.f;
				optimizedClearValue.Color[3] = 0.f;
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

		m_texture = MemoryManager::Get()->CreateResource(
			m_memoryPropertiesFlags.Type,
			&textureDesc,
			m_usageFlags,
			isDepthTarget || allowRenderTarget ? &optimizedClearValue : nullptr);
	}


	Texture::Texture(GUID guid) :
		Resource(guid)
	{
		m_usageFlags = D3D12_RESOURCE_STATE_COPY_DEST;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		bool hasRawData = DataManager::Get()->HasRawData(guid);
		auto textureStream = DataManager::Get()->GetFileStream(guid, hasRawData);

		InitTextureFromFile(textureStream);
	}

	Texture::Texture(
		GUID guid,
		const std::string& file) :
		Resource(guid)
	{
		m_usageFlags = D3D12_RESOURCE_STATE_COPY_DEST;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		const bool hasRawData = DataManager::Get()->HasRawData(file);
		auto textureStream = DataManager::Get()->GetFileStream(file, hasRawData);

		InitTextureFromFile(textureStream);
	}

	void Texture::InitTextureFromFile(std::ifstream& textureStream)
	{
		TextureAssetHeader textureAssetHeader = {};
		textureStream.seekg(0);
		textureStream.read(reinterpret_cast<char*>(&textureAssetHeader), sizeof(TextureAssetHeader));
		uint32_t offset = sizeof(TextureAssetHeader);
		uint32_t ddsMagic = 0;
		textureStream.read(reinterpret_cast<char*>(&ddsMagic), sizeof(uint32_t));
		if (ddsMagic == DDS_MAGIC)
		{
			DDS_HEADER ddsHeader = {};
			textureStream.read(reinterpret_cast<char*>(&ddsHeader), sizeof(DDS_HEADER));
			offset += sizeof(uint32_t) + sizeof(DDS_HEADER);
			if ((ddsHeader.ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == ddsHeader.ddspf.fourCC))
			{
				DDS_HEADER_DXT10 ddsHeaderDxt10 = {};
				textureStream.read(reinterpret_cast<char*>(&ddsHeaderDxt10), sizeof(DDS_HEADER_DXT10));
				offset += sizeof(DDS_HEADER_DXT10);
			}
		}

		m_width = textureAssetHeader.width;
		m_height = textureAssetHeader.height;
		ASSERT((m_width & (m_width - 1)) == 0)
		ASSERT((m_height & (m_height - 1)) == 0)

		m_mipLevels = textureAssetHeader.mipCount;

		switch (textureAssetHeader.format)
		{
		case RGBA8:
			m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case RGBA32:
			m_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case BC1_UNORM:
			m_format = DXGI_FORMAT_BC1_UNORM;
			break;
		case BC6H_UF16:
			m_format = DXGI_FORMAT_BC6H_UF16;
			break;
		default:
			ASSERT(false);
		}

		CreateImageResource(false, false, false, 1, m_mipLevels);
		CreateImageViews();

		MemoryManager::Get()->LoadDataToImage(
			textureStream,
			offset,
			this);
	}


	Texture::Texture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE heapType)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);
		CreateImageResource(false, false, false, 1, 1);
		Texture::CreateImageViews();
	}


	void Texture::CreateImageViews()
	{
		m_resourceView = TextureUtils::CreateSRV(m_format, m_mipLevels, m_texture.Get(), false);
	}

	//void Texture::CreateImageViews(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize)
	//{
	//	ASSERT(!(allowRenderTarget && isDepthTarget ));
	//	ASSERT(!(allowRenderTarget && allowUnorderedAccess));
	//	ASSERT(!(isDepthTarget && allowUnorderedAccess));
	//	ASSERT(arraySize >= 1);
	//	if (allowRenderTarget)
	//	{
	//		D3D12_RENDER_TARGET_VIEW_DESC desc;
	//		desc.Format = m_format;
	//		if (arraySize == 1)
	//		{
	//			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//			desc.Texture2D.MipSlice = 0;
	//			desc.Texture2D.PlaneSlice = 0;
	//			m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get());
	//		}
	//		else if (arraySize == 6)
	//		{
	//			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	//			for (uint32_t i = 0; i < 6; i++)
	//			{
	//				desc.Texture2DArray.ArraySize = 1; // view from single element
	//				desc.Texture2DArray.FirstArraySlice = i;
	//				desc.Texture2DArray.MipSlice = 0;
	//				desc.Texture2DArray.PlaneSlice = 0;
	//				m_resourceViewArray[i] = std::make_unique<ResourceView>(desc, m_texture.Get());
	//			}
	//			desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	//			desc.Texture2DArray.ArraySize = arraySize;
	//			desc.Texture2DArray.FirstArraySlice = 0;
	//			desc.Texture2DArray.MipSlice = 0;
	//			desc.Texture2DArray.PlaneSlice = 0;
	//			m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get());
	//		}
	//		else
	//		{
	//			ASSERT(false); // can we have other rtv arrays?
	//		}
	//	}
	//	else if (isDepthTarget)
	//	{
	//		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	//		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//		depthStencilViewDesc.ViewDimension = arraySize == 1 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	//		if (arraySize == 1)
	//		{
	//			depthStencilViewDesc.Texture2D.MipSlice = 0;
	//		}
	//		else
	//		{
	//			depthStencilViewDesc.Texture2DArray.ArraySize = arraySize;
	//		}
	//		depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	//		m_resourceView = std::make_unique<ResourceView>(depthStencilViewDesc, m_texture.Get());
	//	}
	//	else if (allowUnorderedAccess)
	//	{
	//		ASSERT(arraySize == 1);
	//		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	//		uavDesc.Format = m_format;
	//		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D; // TODO make for arraySize > 1
	//		uavDesc.Texture2D = {
	//			0,
	//			0
	//		};
	//		m_resourceView = std::make_unique<ResourceView>(uavDesc, m_texture.Get());
	//	}
	//	else
	//	{
	//		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	//		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//		desc.Format = m_format;
	//		desc.ViewDimension = arraySize == 1 ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE;
	//		desc.Texture2D = {
	//			0,
	//			1,
	//			0,
	//			0
	//		};
	//		m_resourceView = std::make_unique<ResourceView>(desc, m_texture.Get()); // TODO cube or 2dArray?
	//	}
	//}


	RenderTexture::RenderTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE heapType)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);

		CreateImageResource(true, false, false, 1, 1);
		CreateImageViews();
	}

	RenderTexture::RenderTexture(
		ComPtr<ID3D12Resource> externalResource,
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE heapType)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);
		externalResource->SetName(L"Swapchain Resource");
		m_texture = std::move(externalResource);

		CreateImageViews();
	}

	void RenderTexture::CreateImageViews()
	{
		m_resourceView = TextureUtils::CreateSRV(m_format, 1, m_texture.Get());
		m_renderTargetView = TextureUtils::CreateRTV(m_format, m_texture.Get());
	}


	UAVTexture::UAVTexture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_RESOURCE_STATES usage,
	                       D3D12_HEAP_TYPE heapType)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);

		CreateImageResource(false, false, true, 1, 1);
		CreateImageViews();
	}

	void UAVTexture::CreateImageViews()
	{
		m_resourceView = TextureUtils::CreateSRV(m_format, 1, m_texture.Get());
		m_unorderedAccessView = TextureUtils::CreateUAV(m_format, m_texture.Get());
	}

	UAVRenderTexture::UAVRenderTexture(uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE heapType)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);

		CreateImageResource(true, false, true, 1, 1);
		CreateImageViews();
	}

	void UAVRenderTexture::CreateImageViews()
	{
		m_resourceView = TextureUtils::CreateSRV(m_format, 1, m_texture.Get());
		m_renderTargetView = TextureUtils::CreateRTV(m_format, m_texture.Get());
		m_unorderedAccessView = TextureUtils::CreateUAV(m_format, m_texture.Get());
	}


	DepthTexture::DepthTexture(
		uint32_t width,
		uint32_t height,
		DXGI_FORMAT format,
		D3D12_RESOURCE_STATES usage,
		D3D12_HEAP_TYPE heapType,
		uint32_t arraySize)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_usageFlags = usage;
		m_memoryPropertiesFlags = CD3DX12_HEAP_PROPERTIES(heapType);
		m_arraySize = arraySize;

		CreateImageResource(false, true, false, 1, 1);
		CreateImageViews();
	}

	void DepthTexture::CreateImageViews()
	{
		m_resourceView = TextureUtils::CreateSRV(DXGI_FORMAT_R32_FLOAT, 1, m_texture.Get());
		m_depthStencilView = TextureUtils::CreateDSV(DXGI_FORMAT_D32_FLOAT, m_arraySize, m_texture.Get());
	}
}
