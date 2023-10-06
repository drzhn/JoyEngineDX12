#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "CommonEngineStructs.h"
#include "RenderManager/ComputeDispatcher.h"
#include "RenderManager/Raytracing/RaytracedDDGIDataContainer.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/Buffer.h"
#include "ResourceManager/Buffers/ConstantCpuBuffer.h"
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

	private:
		const RaytracedDDGIDataContainer& m_dataContainer;
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;

		std::unique_ptr<UAVGpuBuffer> m_accelerationTop;
		std::unique_ptr<UAVGpuBuffer> m_accelerationBottom;

		std::unique_ptr<UAVTexture> m_testTexture;

		uint32_t m_raytracedTextureWidth;
		uint32_t m_raytracedTextureHeight;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
