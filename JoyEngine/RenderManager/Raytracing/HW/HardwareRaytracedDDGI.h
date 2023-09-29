#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "RenderManager/ComputeDispatcher.h"
#include "ResourceManager/Buffers/Buffer.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"
#include "ResourceManager/Pipelines/RaytracingPipeline.h"


namespace JoyEngine
{
	class HardwareRaytracedDDGI
	{
	public:
		HardwareRaytracedDDGI();

	private:
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;

		std::unique_ptr<ComputeDispatcher> m_dispatcher;


		std::unique_ptr<Buffer> m_testIndexBuffer;
		std::unique_ptr<Buffer> m_testVertexBuffer;

		std::unique_ptr<UAVGpuBuffer> m_accelerationTop;
		std::unique_ptr<UAVGpuBuffer> m_accelerationBottom;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
