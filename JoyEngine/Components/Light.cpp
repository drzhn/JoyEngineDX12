﻿#include "Light.h"

#include "JoyContext.h"
#include "RenderManager/RenderManager.h"
#include "glm/gtc/type_ptr.hpp"


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
			uint32_t bufferSize = ((sizeof(LightData) - 1) / 256 + 1) * 256; // Device requirement. TODO check this 
			m_lightDataBuffer = std::make_unique<Buffer>(bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
			m_lightDataBufferView = std::make_unique<HeapHandle>(
				D3D12_CONSTANT_BUFFER_VIEW_DESC{
					m_lightDataBuffer->GetBuffer()->GetGPUVirtualAddress(),
					bufferSize
				}
			);
		}
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

	glm::mat4 Light::GetViewMatrix() const
	{
		return m_cameraUnit.GetViewMatrix(m_transform->GetPosition(), m_transform->GetRotation());
	}

	glm::mat4x4 Light::GetProjMatrix() const
	{
		return m_cameraUnit.GetProjMatrix();

		//float s = glm::cos(glm::radians(m_angle )) / glm::sin(glm::radians(m_angle ));
		//float znear = 0.01f;
		//float zfar = m_height;
		//float q = zfar / (zfar - znear);

		//float projArr[16] = {
		//	s, 0, 0, 0,
		//	0, s, 0, 0,
		//	0, 0, q, 1,
		//	0, 0, -q * znear, 0
		//};
		//glm::mat4 proj;

		//memcpy(glm::value_ptr(proj), projArr, sizeof(projArr));
		//return proj;
	}
}
