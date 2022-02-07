#include "Camera.h"

#include "JoyContext.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	void Camera::Enable()
	{
		JoyContext::Render->RegisterCamera(this);
		m_cameraUnit = CameraUnit(JoyContext::Render->GetAspect(),
		                          JoyContext::Render->GetWidth(),
		                          JoyContext::Render->GetHeight(),
		                          60,
		                          0.1f,
		                          1000
		);
		m_enabled = true;
	}

	void Camera::Disable()
	{
		JoyContext::Render->UnregisterCamera(this);
		m_enabled = false;
	}

	void Camera::Update()
	{
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		return m_cameraUnit.GetViewMatrix(m_transform->GetPosition(), m_transform->GetRotation());
	}

	glm::mat4x4 Camera::GetProjMatrix() const
	{
		return m_cameraUnit.GetProjMatrix();
	}
}
