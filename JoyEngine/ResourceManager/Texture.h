#ifndef TEXTURE_H
#define TEXTURE_H

#include "Common/Resource.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Texture final : public Resource
	{
	public:
		explicit Texture();

		explicit Texture(GUID);

		explicit Texture(
			uint32_t width,
			uint32_t height,
			DXGI_FORMAT format,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
		);

		~Texture() override = default;

		void InitializeTexture(const unsigned char* data);
		void InitializeTexture(std::ifstream& stream, uint32_t offset);

		void LoadDataAsync(
			std::ifstream& stream,
			uint64_t offset) const;

		[[nodiscard]] ComPtr<ID3D12Resource> GetImage() noexcept { return m_texture; }


		//[[nodiscard]] VkImageView& GetImageView() noexcept { return m_textureImageView; }

		//[[nodiscard]] VkSampler& GetSampler() noexcept { return m_textureSampler; }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

		//[[nodiscard]] VkImageSubresourceRange* GetSubresourceRange() noexcept { return &m_subresourceRange; }
	private:
		void CreateImage();
		void CreateImageView();
		void CreateImageSampler();

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		DXGI_FORMAT m_format;
		D3D12_RESOURCE_STATES m_usageFlags;
		CD3DX12_HEAP_PROPERTIES m_propertiesFlags;

		ComPtr<ID3D12Resource> m_texture;

		D3D12_CPU_DESCRIPTOR_HANDLE m_textureImageView = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_textureSampler = {};
	};
}

#endif
