#include "Light.h"

#include "JoyContext.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	Light::Light(LightType lightType, float intensity, float radius, float height, float angle):
		m_lightType(lightType),
		m_intensity(intensity),
		m_radius(radius),
		m_height(height),
		m_angle(angle)
	{
	}

	void Light::Enable()
	{
		JoyContext::Render->RegisterLight(this);
	}

	void Light::Disable()
	{
		JoyContext::Render->UnregisterLight(this);
	}
}
