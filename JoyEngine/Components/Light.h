#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

#include "Component.h"
#include "RenderManager/JoyTypes.h"
#include "Common/Color.h"

namespace JoyEngine
{
	class Light : public Component
	{
	public:
		Light(LightType lightType,
		      float intensity,
		      float radius,
		      float height,
		      float angle
		);

		void Enable() override;
		void Disable() override;

		void Update() override
		{
		}

		[[nodiscard]] LightType GetLightType() const noexcept { return m_lightType; }
		[[nodiscard]] float GetIntensity() const noexcept { return m_intensity; }
		[[nodiscard]] float GetRadius() const noexcept { return m_radius; }
		[[nodiscard]] float GetHeight() const noexcept { return m_height; }
		[[nodiscard]] float GetAngle() const noexcept { return m_angle; }

	private:
		LightType m_lightType;
		float m_intensity = 0;
		float m_radius = 0;
		float m_height = 0;
		float m_angle = 0;
	};
}

#endif // LIGHT_H
