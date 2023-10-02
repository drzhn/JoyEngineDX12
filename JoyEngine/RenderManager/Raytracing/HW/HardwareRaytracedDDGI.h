#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "CommonEngineStructs.h"
#include "RenderManager/ComputeDispatcher.h"
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
		HardwareRaytracedDDGI();
		void ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const;

	private:
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;

		std::unique_ptr<ComputeDispatcher> m_dispatcher;


		std::unique_ptr<Buffer> m_testIndexBuffer;
		std::unique_ptr<Buffer> m_testVertexBuffer;

		std::unique_ptr<UAVGpuBuffer> m_accelerationTop;
		std::unique_ptr<UAVGpuBuffer> m_accelerationBottom;

		std::unique_ptr<UAVTexture> m_testTexture;
		ConstantCpuBuffer<RayGenConstantBuffer> m_screenParamsBuffer;
		ConstantCpuBuffer<Color> m_testColorBuffer;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
