#include "Light.h"

#include "JoyContext.h"
#include "RenderManager/RenderManager.h"
#include "glm/gtc/type_ptr.hpp"


namespace JoyEngine
{
	glm::vec3 shadowTransformsForward[6]
	{
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(-1.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, -1.0, 0.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, -1.0)
	};

	glm::vec3 shadowTransformsUp[6]
	{
		glm::vec3(0.0,1.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(0.0, 1.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0)
	};

	Light::Light(LightType lightType, float intensity, float radius, float height, float angle, float ambient):
		m_lightType(lightType),
		m_intensity(intensity),
		m_radius(radius),
		m_height(height),
		m_angle(angle),
		m_ambient(ambient)
	{
		if (lightType == Spot)
		{
			m_shadowmap = std::make_unique<DepthTexture>(
				512,
				512,
				DXGI_FORMAT_R32_TYPELESS,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT);

			m_cameraUnit = CameraUnit(
				1,
				512,
				512,
				m_angle,
				0.1f,
				1000
			);
		}

		if (lightType == Point)
		{
			m_shadowmap = std::make_unique<DepthTexture>(
				512,
				512,
				DXGI_FORMAT_R32_TYPELESS,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT,
				6);

			m_cameraUnit = CameraUnit(
				1,
				512,
				512,
				90,
				0.1f,
				1000
			);
		}


		uint32_t bufferSize = ((sizeof(LightData) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
		m_lightDataBuffer = std::make_unique<Buffer>(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		m_lightDataBufferView = std::make_unique<ResourceView>(
			D3D12_CONSTANT_BUFFER_VIEW_DESC{
				m_lightDataBuffer->GetBuffer()->GetGPUVirtualAddress(),
				bufferSize
			}
		);
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

	void Light::Update()
	{
		const auto ptr = m_lightDataBuffer->GetMappedPtr();
		const auto data = static_cast<LightData*>(ptr->GetMappedPtr());

		// shut up :)
		data->intensity = m_intensity;
		data->radius = m_radius;
		data->height = m_height;
		data->angle = m_angle;

		if (m_lightType == Spot)
		{
			data->view[0] = GetViewMatrix();
			data->proj = GetProjMatrix();
		}

		if (m_lightType == Point)
		{
			data->view[0] = GetCubeViewMatrix(0);
			data->view[1] = GetCubeViewMatrix(1);
			data->view[2] = GetCubeViewMatrix(2);
			data->view[3] = GetCubeViewMatrix(3);
			data->view[4] = GetCubeViewMatrix(4);
			data->view[5] = GetCubeViewMatrix(5);
			data->proj = GetProjMatrix();
		}
	}

	glm::mat4 Light::GetViewMatrix() const
	{
		return m_cameraUnit.GetViewMatrix(m_transform->GetPosition(), m_transform->GetRotation());
	}

	glm::mat4 Light::GetCubeViewMatrix(uint32_t index) const
	{
		const glm::vec3 eye = m_transform->GetPosition();
		return glm::lookAtLH(eye, eye + shadowTransformsForward[index], shadowTransformsUp[index]);
	}

	glm::mat4x4 Light::GetProjMatrix() const
	{
		return m_cameraUnit.GetProjMatrix();
	}
}
