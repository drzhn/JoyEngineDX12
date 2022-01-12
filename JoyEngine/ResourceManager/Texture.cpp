#include "Texture.h"
#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
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
	}

	void Texture::CreateImage()
	{
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = m_format;
		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		// if we want texture to be both shader readable and render target, change this
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateCommittedResource(
				&m_propertiesFlags,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				m_usageFlags,
				nullptr,
				IID_PPV_ARGS(&m_texture))
		);
	}
}
