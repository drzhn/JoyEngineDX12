#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H
#include <memory>

#include "ResourceManager/SharedMaterial.h"


namespace JoyEngine
{
	class HardwareRaytracedDDGI
	{
	public:
		HardwareRaytracedDDGI();
	private:
		std::unique_ptr<RaytracingPipeline> m_raytracingPipeline;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
