#ifndef CUBEMAP_RENDERER_H
#define CUBEMAP_RENDERER_H
#include <memory>

#include "Component.h"
#include "Common/CameraUnit.h"

namespace JoyEngine
{
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
		[[nodiscard]] Texture* GetDepthTexture() const noexcept { return m_depthTexture.get(); }
		[[nodiscard]] glm::mat4 GetCubeViewMatrix(uint32_t index) const;
		[[nodiscard]] glm::mat4x4 GetProjMatrix() const;
		[[nodiscard]] uint32_t GetTextureSize() const noexcept { return m_textureSize; }
	private:
		const uint32_t m_textureSize = 512;

		std::unique_ptr<RenderTexture> m_cubemap;
		std::unique_ptr<Texture> m_depthTexture;
		CameraUnit m_cameraUnit;
	};
}
#endif // CUBEMAP_RENDERER_H
