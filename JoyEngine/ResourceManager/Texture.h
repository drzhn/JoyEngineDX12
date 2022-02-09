#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <fstream>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"
#include "ResourceManager/ResourceView.h"


namespace JoyEngine
{
	enum TextureType
	{
		RGBA_UNORM = 0,
		RGB_FLOAT = 1
	};

	class Texture : public Resource
	{
	public:
		static void InitSamplers();
		static ResourceView* GetTextureSampler();
		static ResourceView* GetDepthPCFSampler();
	private:
		static std::unique_ptr<ResourceView> m_textureSampler;
		static std::unique_ptr<ResourceView> m_depthPCFSampler;
	public:
		explicit Texture() = default;

		explicit Texture(GUID);

		explicit Texture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties,
			bool allowRenderTarget = false,
			bool isDepthTarget = false,
			uint32_t arraySize = 1
		);

		explicit Texture(
			ComPtr<ID3D12Resource> externalResource,
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
		);

		~Texture() override = default;

		[[nodiscard]] ComPtr<ID3D12Resource> GetImage() const noexcept { return m_texture; }

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }

		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }

		[[nodiscard]] DXGI_FORMAT GetFormat() const noexcept { return m_format; }

		[[nodiscard]] ResourceView* GetResourceView() const noexcept { return m_resourceView.get(); }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void CreateImage(bool allowRenderTarget, bool isDepthTarget, uint32_t arraySize);
		void CreateImageView(bool allowRenderTarget, bool isDepthTarget, uint32_t arraySize);

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		DXGI_FORMAT m_format;
		D3D12_RESOURCE_STATES m_usageFlags;
		CD3DX12_HEAP_PROPERTIES m_memoryPropertiesFlags;

		ComPtr<ID3D12Resource> m_texture;
		std::unique_ptr<ResourceView> m_resourceView;
	};

	class RenderTexture final : public Texture
	{
	public:
		explicit RenderTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
		);

		[[nodiscard]] ResourceView* GetAttachmentView() const noexcept { return m_inputAttachmentView.get(); }

	private:
		std::unique_ptr<ResourceView> m_inputAttachmentView; // additional view for using this texture as input attachment
	};

	class DepthTexture final : public Texture
	{
	public:
		explicit DepthTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties,
			uint32_t arraySize = 1
		);

		[[nodiscard]] ResourceView* GetAttachmentView() const noexcept { return m_inputAttachmentView.get(); }

	private:
		std::unique_ptr<ResourceView> m_inputAttachmentView; // additional view for using this texture as input attachment
	};
}

#endif
