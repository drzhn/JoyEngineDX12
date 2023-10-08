#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "RenderManager/GBuffer.h"
#include "RenderManager/Raytracing/AbstractRaytracedDDGI.h"
#include "RenderManager/Raytracing/RaytracedDDGIDataContainer.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"
#include "ResourceManager/Pipelines/RaytracingPipeline.h"


namespace JoyEngine
{
	class HardwareRaytracedDDGI : public AbstractRaytracedDDGI
	{
	public:
		HardwareRaytracedDDGI(
			const RaytracedDDGIDataContainer& dataContainer,
			DXGI_FORMAT mainColorFormat,
			DXGI_FORMAT swapchainFormat,
			uint32_t width,
			uint32_t height
		);
		void UploadSceneData() override;
		void ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const override;
		void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const override;
		void GenerateProbeIrradiance(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const override;
		void DebugDrawProbes(
			ID3D12GraphicsCommandList* commandList, 
			uint32_t frameIndex, 
			const ViewProjectionMatrixData* viewProjectionMatrixData) const override;

		[[nodiscard]] UAVGbuffer* GetGBuffer() const override { return m_gbuffer.get(); }
		[[nodiscard]] RenderTexture* GetShadedRenderTexture() const override { return m_shadedRenderTexture.get(); }
		[[nodiscard]] uint32_t GetRaytracedTextureWidth() const noexcept override { return m_raytracedTextureWidth; }
		[[nodiscard]] uint32_t GetRaytracedTextureHeight() const noexcept override { return m_raytracedTextureHeight; }
		[[nodiscard]] UAVTexture* GetProbeIrradianceTexture() const override { return m_probeIrradianceTexture.get(); }
		[[nodiscard]] UAVTexture* GetProbeDepthTexture() const override { return m_probeDepthTexture.get(); }
	private:
		const RaytracedDDGIDataContainer& m_dataContainer;
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;

		std::unique_ptr<UAVGpuBuffer> m_accelerationTop;
		std::unique_ptr<UAVGpuBuffer> m_accelerationBottom;

		std::unique_ptr<UAVGbuffer> m_gbuffer;
		std::unique_ptr<RenderTexture> m_shadedRenderTexture;
		std::unique_ptr<UAVTexture> m_probeIrradianceTexture; // octohedral irradince per-probe storage
		std::unique_ptr<UAVTexture> m_probeDepthTexture; // octohedral depth per-probe storage

		uint32_t m_raytracedTextureWidth;
		uint32_t m_raytracedTextureHeight;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
