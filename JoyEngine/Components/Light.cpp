#include "Light.h"


#include "imgui.h"
#include "MeshRenderer.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "RenderManager/LightSystems/ILightSystem.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"


namespace JoyEngine
{
	//Light::Light(LightType lightType, float intensity, float radius, float height, float angle, float ambient):
	//	m_lightType(lightType),
	//	m_intensity(intensity),
	//	m_radius(radius),
	//	m_height(height),
	//	m_angle(angle),
	//	m_ambient(ambient)
	//{
	//	if (lightType == Spot)
	//	{
	//		m_shadowmap = std::make_unique<DepthTexture>(
	//			512,
	//			512,
	//			DXGI_FORMAT_R32_TYPELESS,
	//			D3D12_RESOURCE_STATE_GENERIC_READ,
	//			D3D12_HEAP_TYPE_DEFAULT);

	//		m_cameraUnit = CameraUnit(
	//			1,
	//			512,
	//			512,
	//			m_angle,
	//			0.1f,
	//			1000
	//		);
	//	}

	//	if (lightType == Point)
	//	{
	//		m_shadowmap = std::make_unique<DepthTexture>(
	//			512,
	//			512,
	//			DXGI_FORMAT_R32_TYPELESS,
	//			D3D12_RESOURCE_STATE_GENERIC_READ,
	//			D3D12_HEAP_TYPE_DEFAULT,
	//			6);

	//		m_cameraUnit = CameraUnit(
	//			1,
	//			512,
	//			512,
	//			90,
	//			0.1f,
	//			1000
	//		);
	//	}


	//	uint32_t bufferSize = ((sizeof(LightData) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
	//	m_lightDataBuffer = std::make_unique<Buffer>(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
	//	m_lightDataBufferView = std::make_unique<ResourceView>(
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC{
	//			m_lightDataBuffer->GetBufferResource()->GetGPUVirtualAddress(),
	//			bufferSize
	//		}
	//	);
	//}

	//void Light::Enable()
	//{
	//	if (m_lightType == Direction)
	//	{
	//		RenderManager::Get()->RegisterDirectionLight(this);
	//	}
	//	else
	//	{
	//		RenderManager::Get()->RegisterLight(this);
	//	}
	//}

	//void Light::Disable()
	//{
	//	if (m_lightType == Direction)
	//	{
	//		RenderManager::Get()->UnregisterDirectionLight(this);
	//	}
	//	else
	//	{
	//		RenderManager::Get()->UnregisterLight(this);
	//	}
	//}

	//void Light::Update()
	//{
	//	const auto ptr = m_lightDataBuffer->GetPtr();
	//	const auto data = static_cast<LightData*>(ptr->GetPtr());

	//	// shut up :)
	//	data->intensity = m_intensity;
	//	data->radius = m_radius;
	//	data->height = m_height;
	//	data->angle = m_angle;

	//	if (m_lightType == Spot)
	//	{
	//		data->view[0] = GetViewMatrix();
	//		data->proj = GetProjMatrix();
	//	}

	//	if (m_lightType == Point)
	//	{
	//		data->view[0] = GetCubeViewMatrix(0);
	//		data->view[1] = GetCubeViewMatrix(1);
	//		data->view[2] = GetCubeViewMatrix(2);
	//		data->view[3] = GetCubeViewMatrix(3);
	//		data->view[4] = GetCubeViewMatrix(4);
	//		data->view[5] = GetCubeViewMatrix(5);
	//		data->proj = GetProjMatrix();
	//	}
	//}

	//jmath::mat4 Light::GetViewMatrix() const
	//{
	//	return m_cameraUnit.GetViewMatrix(m_transform->GetPosition(), m_transform->GetRotation());
	//}

	//jmath::mat4 Light::GetCubeViewMatrix(uint32_t index) const
	//{
	//	const jmath::vec3 eye = m_transform->GetPosition();
	//	return jmath::lookAtLH(eye, eye + shadowTransformsForward[index], shadowTransformsUp[index]);
	//}

	//jmath::mat4x4 Light::GetProjMatrix() const
	//{
	//	return m_cameraUnit.GetProjMatrix();
	//}

	uint32_t PackColor(float color[4])
	{
		uint32_t ret = 0;
		ret |= static_cast<uint32_t>(color[0] * 255 + 0.5f) << 24;
		ret |= static_cast<uint32_t>(color[1] * 255 + 0.5f) << 16;
		ret |= static_cast<uint32_t>(color[2] * 255 + 0.5f) << 8;
		ret |= static_cast<uint32_t>(color[3] * 255 + 0.5f) << 0;

		return ret;
	}

	void UnpackColor(float color[4], uint32_t packedColor)
	{
		color[0] = static_cast<float>((packedColor >> 24) & 255) / 255.0f;
		color[1] = static_cast<float>((packedColor >> 16) & 255) / 255.0f;
		color[2] = static_cast<float>((packedColor >> 8) & 255) / 255.0f;
		color[3] = static_cast<float>((packedColor >> 0) & 255) / 255.0f;
	}

	DirectionalLight::DirectionalLight(GameObject& go, ILightSystem& lightSystem, float intensity, float ambient, float color[4])
		: LightBase(
			  go,
			  lightSystem,
			  lightSystem.RegisterDirectionalLight()),
		  m_cameraUnit(1, 50, 0.1f, 1000.0f)
	{
		DirectionalLightInfo& lightData = m_lightSystem.GetDirectionalLightData();
		lightData.ambient = ambient;
		lightData.intensity = intensity;
		lightData.packedColor = PackColor(color);
	}


	void DirectionalLight::Enable()
	{
		m_currentAngle = 30;
	}

	void DirectionalLight::Disable()
	{
		m_lightSystem.UnregisterDirectionalLight();
	}

	void DirectionalLight::Update()
	{
		// TODO this should be controlled by behaviour class
		m_gameObject.GetTransform().SetRotation(jmath::vec3(m_currentAngle, 180.f, 0.f));
		m_gameObject.GetTransform().SetPosition(jmath::vec3(
			0.f,
			70 * jmath::cos(jmath::toRadians(90 - m_currentAngle)),
			70 * jmath::sin(jmath::toRadians(90 - m_currentAngle))));

		DirectionalLightInfo& lightData = m_lightSystem.GetDirectionalLightData();

		lightData.direction = m_gameObject.GetTransform().GetForward();
		lightData.proj = m_cameraUnit.GetProjMatrix();
		lightData.view = m_cameraUnit.GetViewMatrix(
			m_gameObject.GetTransform().GetXPosition(),
			m_gameObject.GetTransform().GetRotation());

		float tempColor[4];
		UnpackColor(tempColor, lightData.packedColor);
		ImGui::SetNextWindowPos({0, 300}); // move to DrawGui()?...
		ImGui::SetNextWindowSize({300, 100}); // move to DrawGui()?...
		{
			ImGui::Begin("Directional Light:");
			ImGui::SliderFloat("Angle", &m_currentAngle, 0.f, 90.f);
			//ImGui::SliderFloat("Bias", &lightData.bias, 0.f, 0.002f);
			ImGui::ColorEdit3("Color", tempColor);
			ImGui::End();
		}
		lightData.packedColor = PackColor(tempColor);
	}

	PointLight::PointLight(uint32_t frameIndex, GameObject& go, ILightSystem& lightSystem, float radius, float intensity, float color[4]):
		LightBase(
			go,
			lightSystem,
			lightSystem.RegisterLight(this))
	{
		auto& lightInfo = m_lightSystem.GetLightInfo(frameIndex, m_lightIndex);
		lightInfo.radius = radius;
		lightInfo.intensity = intensity;
		lightInfo.packedColor = PackColor(color);
		lightInfo.transformIndex = GetGameObject().GetTransform().GetTransformIndex();
	}

	void PointLight::Enable()
	{
	}

	void PointLight::Disable()
	{
	}

	void PointLight::Update()
	{
	}
}
