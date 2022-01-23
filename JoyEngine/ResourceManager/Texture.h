#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <fstream>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

#include "Common/Resource.h"
#include "ResourceManager/HeapHandle.h"


namespace JoyEngine
{
	class Texture : public Resource
	{
	public:
		explicit Texture() = default;

		explicit Texture(GUID);

		explicit Texture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
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

		[[nodiscard]] DXGI_FORMAT GetFormat() const noexcept { return m_format; }

		[[nodiscard]] HeapHandle* GetResourceView() const noexcept { return m_resourceView.get(); }

		[[nodiscard]] HeapHandle* GetSampleView() const noexcept { return m_samplerView.get(); }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void CreateImage();
		void CreateImageView();
		void CreateImageSampler();

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		DXGI_FORMAT m_format;
		D3D12_RESOURCE_STATES m_usageFlags;
		CD3DX12_HEAP_PROPERTIES m_memoryPropertiesFlags;

		ComPtr<ID3D12Resource> m_texture;

		std::unique_ptr<HeapHandle> m_resourceView;
		std::unique_ptr<HeapHandle> m_samplerView;
	};

	class RenderTexture final: public Texture
	{
	public:
		explicit RenderTexture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
		);

		[[nodiscard]] HeapHandle* GetAttachmentView() const noexcept { return m_inputAttachmentView.get(); }

	private:
		std::unique_ptr<HeapHandle> m_inputAttachmentView; // additional view for using this texture as input attachment
	};
}

#endif
