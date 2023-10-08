#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "RenderManager/GBuffer.h"
#include "RenderManager/Raytracing/RaytracedDDGIDataContainer.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"
#include "ResourceManager/Pipelines/RaytracingPipeline.h"


namespace JoyEngine
{
	class HardwareRaytracedDDGI
	{
	public:
		HardwareRaytracedDDGI(
			const RaytracedDDGIDataContainer& dataContainer,
			DXGI_FORMAT mainColorFormat,
			DXGI_FORMAT swapchainFormat,
			uint32_t width,
			uint32_t height
		);
		void UploadSceneData();
		void ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const;
		void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const;

		[[nodiscard]] UAVGbuffer* GetGBuffer() const { return m_gbuffer.get(); }
		[[nodiscard]] RenderTexture* GetShadedRenderTexture() const { return m_shadedRenderTexture.get(); }
		[[nodiscard]] uint32_t GetRaytracedTextureWidth() const noexcept { return m_raytracedTextureWidth; }
		[[nodiscard]] uint32_t GetRaytracedTextureHeight() const noexcept { return m_raytracedTextureHeight; }

	private:
		const RaytracedDDGIDataContainer& m_dataContainer;
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;

		std::unique_ptr<UAVGpuBuffer> m_accelerationTop;
		std::unique_ptr<UAVGpuBuffer> m_accelerationBottom;

		std::unique_ptr<UAVGbuffer> m_gbuffer;
		std::unique_ptr<RenderTexture> m_shadedRenderTexture;

		uint32_t m_raytracedTextureWidth;
		uint32_t m_raytracedTextureHeight;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
