#include "Camera.h"

#include "JoyContext.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	void Camera::Enable()
	{
		JoyContext::Render->RegisterCamera(this);
		m_aspect = JoyContext::Render->GetAspect();
		m_width = JoyContext::Render->GetWidth();
		m_height = JoyContext::Render->GetHeight();
		m_fov = 60;
		m_near = 0.1f;
		m_far = 1000;
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

	glm::mat4 Camera::GetProjMatrix() const
	{
		glm::mat4 proj = glm::perspectiveFovLH_ZO(glm::radians(m_fov), m_width, m_height, m_near, m_far);
		//proj[1][1] *= -1;
		//proj[0][0] *= -1;
		return proj;
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		const glm::vec3 eye =   m_transform->GetPosition();
		const glm::vec3 center = m_transform->GetPosition() + m_transform->GetRotation() * glm::vec3(0, 0, 1);
		const glm::vec3 up = m_transform->GetRotation() * glm::vec3(0, 1, 0);

		//const glm::vec3 eye = glm::vec3(0, 3, -3); // m_transform->GetPosition();
		//const glm::vec3 center = glm::vec3(0, 0, 0); //m_transform->GetPosition() + m_transform->GetRotation() * glm::vec3(0, 0, 1);
		//const glm::vec3 up = m_transform->GetRotation() * glm::vec3(0, 1, 0);

		return glm::lookAtLH(eye, center, up);
	}
}
