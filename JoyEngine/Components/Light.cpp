#include "Light.h"

#include "JoyContext.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	Light::Light(LightType lightType, float intensity, float radius, float height, float angle, float ambient):
		m_lightType(lightType),
		m_intensity(intensity),
		m_radius(radius),
		m_height(height),
		m_angle(angle),
		m_ambient(ambient)
	{
	}

	void Light::Enable()
	{
		if (m_lightType == Direction)
		{
			JoyContext::Render->RegisterDirectionLight(this);
		}
		else
		{
			JoyContext::Render->RegisterLight(this);
		}
	}

	void Light::Disable()
	{
		if (m_lightType == Direction)
		{
			JoyContext::Render->UnregisterDirectionLight(this);
		}
		else
		{
			JoyContext::Render->UnregisterLight(this);
		}
	}
}
