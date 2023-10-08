#ifndef ABSTRACT_RAYTRACED_DDGI_H
#define ABSTRACT_RAYTRACED_DDGI_H
#include <cstdint>

#include "CommonEngineStructs.h"
#include "RenderManager/GBuffer.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	class AbstractRaytracedDDGI
	{
	public:
		virtual ~AbstractRaytracedDDGI() = default;

		virtual void UploadSceneData() = 0;
		virtual void ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const = 0;
		virtual void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const = 0;
		virtual void GenerateProbeIrradiance(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const = 0;
		virtual void DebugDrawProbes(
			ID3D12GraphicsCommandList* commandList,
			uint32_t frameIndex,
			const ViewProjectionMatrixData* viewProjectionMatrixData) const = 0;

		[[nodiscard]] virtual UAVGbuffer* GetGBuffer() const = 0;
		[[nodiscard]] virtual RenderTexture* GetShadedRenderTexture() const = 0;
		[[nodiscard]] virtual uint32_t GetRaytracedTextureWidth() const noexcept = 0;
		[[nodiscard]] virtual uint32_t GetRaytracedTextureHeight() const noexcept = 0;
		[[nodiscard]] virtual UAVTexture* GetProbeIrradianceTexture() const = 0;
		[[nodiscard]] virtual UAVTexture* GetProbeDepthTexture() const = 0;
	};
}
#endif // ABSTRACT_RAYTRACED_DDGI_H
