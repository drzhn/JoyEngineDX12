#ifndef SSAO_H
#define SSAO_H

#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/Texture.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class RenderTexture;
	class Buffer;
	class ResourceView;

	class SSAO
	{
	public:
		SSAO() = delete;
		SSAO(uint32_t width, uint32_t height, DXGI_FORMAT format);
		~SSAO() = default;
		[[nodiscard]] ID3D12Resource* GetRenderResource() const { return m_ssaoRenderTarget->GetImage().Get(); }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRenderHandle() const { return m_ssaoRenderTarget->GetResourceView()->GetHandle(); }
		[[nodiscard]] ResourceView* GetOffsetBufferView() const { return m_offsetBufferView.get(); }
		[[nodiscard]] ResourceView* GetRandomNoiseTextureView() const { return m_randomColorTexture->GetResourceView(); }

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }

	private:
		uint32_t m_width;
		uint32_t m_height;
		std::unique_ptr<Buffer> m_offsetBuffer;
		std::unique_ptr<ResourceView> m_offsetBufferView;
		std::unique_ptr<RenderTexture> m_ssaoRenderTarget;
		ResourceHandle<Texture> m_randomColorTexture;
	};
}

#endif // SSAO_H
