#ifndef LIGHT_H
#define LIGHT_H

#include "Component.h"
#include "Common/CameraUnit.h"
#include "RenderManager/JoyTypes.h"
#include "Common/Color.h"

namespace JoyEngine
{
	class Texture;

	class Light : public Component
	{
	public:
		Light(LightType lightType,
		      float intensity,
		      float radius,
		      float height,
		      float angle,
		      float ambient
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
		[[nodiscard]] float GetAmbient() const noexcept { return m_ambient; }
		[[nodiscard]] Texture* GetShadowmap() const noexcept { return m_shadowmap.get(); }

		[[nodiscard]] glm::mat4x4 GetViewMatrix() const;
		[[nodiscard]] glm::mat4x4 GetProjMatrix() const;
	private:
		LightType m_lightType;
		float m_intensity = 0;
		float m_radius = 0;
		float m_height = 0;
		float m_angle = 0;
		float m_ambient = 0;

		std::unique_ptr<Texture> m_shadowmap;
		CameraUnit m_cameraUnit;
	};
}

#endif // LIGHT_H
