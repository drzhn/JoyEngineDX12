#ifndef HARDWARE_RAYTRACED_DDGI_H
#define HARDWARE_RAYTRACED_DDGI_H

#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/Shader.h"

namespace JoyEngine
{
	class HardwareRaytracedDDGI
	{
	public:
		HardwareRaytracedDDGI();
	private:
		ResourceHandle<Shader> m_testRaytracingShader;
	};
}

#endif // HARDWARE_RAYTRACED_DDGI_H
