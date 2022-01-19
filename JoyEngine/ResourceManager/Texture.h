#ifndef TEXTURE_H
#define TEXTURE_H

#include "Common/Resource.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <fstream>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Texture final : public Resource
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

		//void InitializeTexture(const unsigned char* data);
		//void InitializeTexture(std::ifstream& stream, uint32_t offset);

		//void LoadDataAsync(
		//	std::ifstream& stream,
		//	uint64_t offset) const;

		[[nodiscard]] ComPtr<ID3D12Resource> GetImage() noexcept { return m_texture; }

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetImageViewCPUHandle() const noexcept { return m_textureImageView; }

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetSampleCPUHandle() const noexcept { return m_textureSampler; }

		[[nodiscard]] ID3D12DescriptorHeap* GetImageViewHeap() const noexcept { return m_resourceViewDescriptorHeap.Get(); }

		[[nodiscard]] ID3D12DescriptorHeap* GetSampleHeap() const noexcept { return m_samplerDescriptorHeap.Get(); }

		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetImageViewGPUHandle() const noexcept { return m_resourceViewDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetSampleGPUHandle() const noexcept { return m_samplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart(); }

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

		ComPtr<ID3D12DescriptorHeap> m_resourceViewDescriptorHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_textureImageView = {};

		ComPtr<ID3D12DescriptorHeap> m_samplerDescriptorHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_textureSampler = {};

		std::ifstream m_textureStream;
	};
}

#endif
