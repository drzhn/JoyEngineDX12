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

		void SetDirection(bool isHorizontal) const;

		[[nodiscard]] ID3D12Resource* GetRenderResource() const { return m_ssaoRenderTarget->GetImage().Get(); }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetRenderHandle() const { return m_ssaoRenderTarget->GetResourceView()->GetCPUHandle(); }
		[[nodiscard]] ID3D12Resource* GetCopyResource() const { return m_ssaoCopyResource->GetImage().Get(); }

		[[nodiscard]] ResourceView* GetSSAODataBufferView() const { return m_ssaoDataBufferView.get(); }
		[[nodiscard]] ResourceView* GetRandomNoiseTextureView() const { return m_randomColorTexture->GetResourceView(); }
		[[nodiscard]] ResourceView* GetSSAOTextureView() const { return m_ssaoRenderTarget->GetSrv(); }
		[[nodiscard]] ResourceView* GetCopyResourceTextureView() const { return m_ssaoCopyResource->GetResourceView(); }

		[[nodiscard]] uint32_t GetWidth() const noexcept { return m_width; }
		[[nodiscard]] uint32_t GetHeight() const noexcept { return m_height; }

	private:
		uint32_t m_width;
		uint32_t m_height;
		std::unique_ptr<Buffer> m_ssaoDataBuffer;
		std::unique_ptr<ResourceView> m_ssaoDataBufferView;

		std::unique_ptr<RenderTexture> m_ssaoRenderTarget;
		std::unique_ptr<Texture> m_ssaoCopyResource;
		ResourceHandle<Texture> m_randomColorTexture;
	};
}

#endif // SSAO_H
