#ifndef LIGHT_H
#define LIGHT_H

#include "Component.h"
#include "Common/CameraUnit.h"
#include "CommonEngineStructs.h"

namespace JoyEngine
{
	class ILightSystem;
	class Texture;
	class DepthTexture;

	class LightBase : public Component
	{
	public:
		LightBase() = delete;

		[[nodiscard]] uint32_t GetIndex() const noexcept { return m_lightIndex; }

	protected:
		explicit LightBase(GameObject& go, ILightSystem& lightSystem, uint32_t lightIndex):
			Component(go),
			m_lightSystem(lightSystem),
			m_lightIndex(lightIndex)
		{
		}

		ILightSystem& m_lightSystem;

		const uint32_t m_lightIndex;
	};

	class DirectionalLight : public LightBase
	{
	public:
		explicit DirectionalLight(GameObject& go, ILightSystem& lightSystem, float intensity, float ambient);

		void Enable() override;

		void Disable() override;

		void Update() override;

		float* GetCurrentAnglePtr() { return &m_currentAngle; }

	private:
		CameraUnit m_cameraUnit;
		float m_currentAngle = 0;
	};

	class PointLight: public LightBase
	{
	public:
		explicit PointLight(GameObject& go, ILightSystem& lightSystem, float radius, float intensity, float color[4]);
		
		void Enable() override;
		void Disable() override;
		void Update() override;
	};

	//class Light : public Component
	//{
	//public:
	//	Light(LightType lightType,
	//	      float intensity,
	//	      float radius,
	//	      float height,
	//	      float angle,
	//	      float ambient
	//	);

	//	void Enable() override;
	//	void Disable() override;

	//	void Update() override;

	//	[[nodiscard]] LightType GetLightType() const noexcept { return m_lightType; }
	//	[[nodiscard]] float GetIntensity() const noexcept { return m_intensity; }
	//	[[nodiscard]] float GetRadius() const noexcept { return m_radius; }
	//	[[nodiscard]] float GetHeight() const noexcept { return m_height; }
	//	[[nodiscard]] float GetAngle() const noexcept { return m_angle; }
	//	[[nodiscard]] float GetAmbient() const noexcept { return m_ambient; }
	//	[[nodiscard]] DepthTexture* GetShadowmap() const noexcept { return m_shadowmap.get(); }

	//	[[nodiscard]] glm::mat4x4 GetViewMatrix() const;
	//	[[nodiscard]] glm::mat4 GetCubeViewMatrix(uint32_t index) const;
	//	[[nodiscard]] glm::mat4x4 GetProjMatrix() const;

	//	[[nodiscard]] ResourceView* GetLightDataBufferView() const noexcept { return m_lightDataBufferView.get(); }
	//private:
	//	LightType m_lightType;
	//	float m_intensity = 0;
	//	float m_radius = 0;
	//	float m_height = 0;
	//	float m_angle = 0;
	//	float m_ambient = 0;

	//	CameraUnit m_cameraUnit;
	//	std::unique_ptr<DepthTexture> m_shadowmap;

	//	std::unique_ptr<Buffer> m_lightDataBuffer;
	//	std::unique_ptr<ResourceView> m_lightDataBufferView;
	//};
}

#endif // LIGHT_H
