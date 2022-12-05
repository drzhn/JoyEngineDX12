#ifndef LIGHT_H
#define LIGHT_H

#include "Component.h"
#include "Common/CameraUnit.h"
#include "CommonEngineStructs.h"
#include "RenderManager/IRenderManager.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/DynamicCpuBuffer.h"
#include "ResourceManager/ResourceView.h"

namespace JoyEngine
{
	class Texture;
	class DepthTexture;

	template<typename LightDataT>
	class Light : public Component
	{
	public:
		Light() = delete;

	protected:
		explicit Light(IRenderManager* renderManager):
		m_renderManager(renderManager),
			m_lightDataBuffer(std::make_unique<DynamicCpuBuffer<LightDataT>>(renderManager->GetFrameCount()))
		{
			
		};
		IRenderManager* m_renderManager;
		std::unique_ptr<DepthTexture> m_shadowmap;

		LightDataT m_lightData;
		std::unique_ptr<DynamicCpuBuffer<LightDataT>> m_lightDataBuffer;
	};

	class DirectionalLight : public Light<DirectionLightData>
	{
	public:
		explicit DirectionalLight(IRenderManager* renderManager, float intensity, float ambient);

		void Enable() override;

		void Disable() override;

		void Update() override;
	private:
		CameraUnit m_cameraUnit;
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
