﻿#ifndef TEXTURE_H
#define TEXTURE_H

#include <array>
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
		static ResourceView* GetDepthSampler();
		static ResourceView* GetPointSampler();
	private:
		static std::unique_ptr<ResourceView> m_textureSampler;
		static std::unique_ptr<ResourceView> m_depthPCFSampler;
		static std::unique_ptr<ResourceView> m_depthSampler;
		static std::unique_ptr<ResourceView> m_pointSampler;
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
			bool allowUnorderedAccess = false,
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
		[[nodiscard]] std::array<std::unique_ptr<ResourceView>, 6>& GetResourceViewArray()
		{
			return m_resourceViewArray;
		}

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void CreateImage(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize, uint32_t mipLevels = 1);
		void CreateImageView(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize);

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		DXGI_FORMAT m_format;
		D3D12_RESOURCE_STATES m_usageFlags;
		CD3DX12_HEAP_PROPERTIES m_memoryPropertiesFlags;

		ComPtr<ID3D12Resource> m_texture;
		// TODO make 2 separate classes: texture and texture+view class
		std::unique_ptr<ResourceView> m_resourceView; // for single textures
		std::array<std::unique_ptr<ResourceView>, 6> m_resourceViewArray; // for cubemap rtvs
	};

	class RenderTexture final : public Texture
	{
	public:
		explicit RenderTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties,
			uint32_t arraySize = 1
		);

		[[nodiscard]] ResourceView* GetSrv() const noexcept { return m_inputAttachmentView.get(); }

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

		[[nodiscard]] ResourceView* GetSrv() const noexcept { return m_inputAttachmentView.get(); }

	private:
		std::unique_ptr<ResourceView> m_inputAttachmentView; // additional view for using this texture as input attachment
	};

	class UAVTexture final : public Texture
	{
	public:
		explicit UAVTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties,
			uint32_t arraySize = 1
		);

		[[nodiscard]] ResourceView* GetSrv() const noexcept { return m_inputAttachmentView.get(); }

	private:
		std::unique_ptr<ResourceView> m_inputAttachmentView; // additional view for using this texture as input attachment
	};
}

#endif
