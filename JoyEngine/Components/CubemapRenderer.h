#ifndef CUBEMAP_RENDERER_H
#define CUBEMAP_RENDERER_H
#include <memory>

#include "Component.h"
#include "Common/CameraUnit.h"

namespace JoyEngine
{
	class Buffer;
	class ResourceView;
	class Texture;
	class RenderTexture;

	class CubemapRenderer : public Component
	{
	public:
		CubemapRenderer();

		void Enable() override;
		void Disable() override;
		void Update() override;

		[[nodiscard]] RenderTexture* GetCubemapTexture() const noexcept { return m_cubemap.get(); }
		[[nodiscard]] RenderTexture* GetCubemapConvolutedTexture() const noexcept { return m_cubemapConvoluted.get(); }
		[[nodiscard]] Texture* GetDepthTexture() const noexcept { return m_depthTexture.get(); }
		[[nodiscard]] glm::mat4 GetCubeViewMatrix(uint32_t index) const;
		[[nodiscard]] glm::mat4x4 GetProjMatrix() const;
		[[nodiscard]] uint32_t GetTextureSize() const noexcept { return m_textureSize; }
		[[nodiscard]] uint32_t GetConvolutedTextureSize() const noexcept { return m_convolutedTextureSize; }

		[[nodiscard]] ResourceView* GetConvolutionConstantsBufferView() const noexcept { return m_convolutionConstantsBufferView.get(); }

	private:
		const uint32_t m_textureSize = 512;
		const uint32_t m_convolutedTextureSize = 128;

		std::unique_ptr<RenderTexture> m_cubemap;
		std::unique_ptr<RenderTexture> m_cubemapConvoluted;
		std::unique_ptr<Texture> m_depthTexture;
		CameraUnit m_cameraUnit;

		std::unique_ptr<Buffer> m_convolutionConstantsDataBuffer;
		std::unique_ptr<ResourceView> m_convolutionConstantsBufferView;
	};
}
#endif // CUBEMAP_RENDERER_H
