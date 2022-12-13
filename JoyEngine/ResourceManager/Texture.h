#ifndef TEXTURE_H
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
	class EngineSamplersProvider
	{
	public:
		static void InitSamplers();
		static ResourceView* GetLinearWrapSampler();
		static ResourceView* GetLinearClampSampler();
		static ResourceView* GetDepthPCFSampler();
		static ResourceView* GetLinearBorderWhiteSampler();
		static ResourceView* GetPointClampSampler();
	private:
		static std::unique_ptr<ResourceView> m_linearWrapSampler;
		static std::unique_ptr<ResourceView> m_linearClampSampler;
		static std::unique_ptr<ResourceView> m_depthPCFSampler;
		static std::unique_ptr<ResourceView> m_linearBorderWhiteSampler;
		static std::unique_ptr<ResourceView> m_pointClampSampler;
	};

	class AbstractTextureResource
	{
	public :
		[[nodiscard]] ComPtr<ID3D12Resource> GetImageResource() const noexcept { return m_texture; }

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }

		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }

		[[nodiscard]] DXGI_FORMAT GetFormat() const noexcept { return m_format; }
	protected:
		void CreateImageResource(bool allowRenderTarget, bool isDepthTarget, bool allowUnorderedAccess, uint32_t arraySize, uint32_t mipLevels = 1);

	protected:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
		D3D12_RESOURCE_STATES m_usageFlags = D3D12_RESOURCE_STATE_COMMON;
		CD3DX12_HEAP_PROPERTIES m_memoryPropertiesFlags;

		ComPtr<ID3D12Resource> m_texture;
	};

	class AbstractSingleTexture : public AbstractTextureResource
	{
	public:
		[[nodiscard]] ResourceView* GetSRV() const noexcept { return m_resourceView.get(); }

	protected:
		virtual void CreateImageViews() = 0;

	protected:
		std::unique_ptr<ResourceView> m_resourceView;
	};

	class AbstractArrayTexture : public AbstractTextureResource
	{
	public:
		[[nodiscard]] std::vector<std::unique_ptr<ResourceView>>& GetResourceViewArray()
		{
			return m_resourceViewArray;
		}

	protected:
		virtual void CreateImageViews() = 0;

	private:
		std::vector<std::unique_ptr<ResourceView>> m_resourceViewArray;
	};

	class Texture : public Resource, public AbstractSingleTexture
	{
	public:
		explicit Texture() = default;

		explicit Texture(GUID);
		explicit Texture(GUID guid, const std::string& file);

		explicit Texture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType
		);

		~Texture() override = default;

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void InitTextureFromFile(std::ifstream& textureStream);
		void CreateImageViews() override;

	private:
		uint32_t m_mipLevels = 0;
	};

	class RenderTexture final : public AbstractSingleTexture
	{
	public:
		explicit RenderTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType
		);

		// I use this only for creating texture from system back buffer
		explicit RenderTexture(
			ComPtr<ID3D12Resource> externalResource,
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType
		);

		[[nodiscard]] ResourceView* GetRTV() const noexcept { return m_renderTargetView.get(); }
	protected:
		void CreateImageViews() override;
	private:
		std::unique_ptr<ResourceView> m_renderTargetView;
	};
	
	class UAVTexture final : public AbstractSingleTexture
	{
	public:
		explicit UAVTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType
		);
		[[nodiscard]] ResourceView* GetUAV() const noexcept { return m_unorderedAccessView.get(); }
	protected:
		void CreateImageViews() override;
	private:
		std::unique_ptr<ResourceView> m_unorderedAccessView;
	};

	// I don't like idea of diamond inheritance
	class UAVRenderTexture final : public AbstractSingleTexture
	{
	public:
		explicit UAVRenderTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType
		);
		[[nodiscard]] ResourceView* GetUAV() const noexcept { return m_unorderedAccessView.get(); }
		[[nodiscard]] ResourceView* GetRTV() const noexcept { return m_renderTargetView.get(); }
	protected:
		void CreateImageViews() override;
	private:
		std::unique_ptr<ResourceView> m_renderTargetView;
		std::unique_ptr<ResourceView> m_unorderedAccessView;
	};

	class DepthTexture final : public AbstractSingleTexture
	{
	public:
		explicit DepthTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE heapType,
			uint32_t arraySize = 1
		);

		[[nodiscard]] ResourceView* GetDSV() const noexcept { return m_depthStencilView.get(); }
	protected:
		void CreateImageViews() override;
	private:
		uint32_t m_arraySize; // we will use depth array texture as single view for rendering
		std::unique_ptr<ResourceView> m_depthStencilView;
	};
}

#endif
