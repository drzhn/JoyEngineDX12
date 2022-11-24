#include "Camera.h"

#include <glm/gtx/matrix_decompose.hpp>

#include "RenderManager/IRenderManager.h"
#include "SceneManager/Transform.h"

#define CAMERA_FOV 60.f
#define CAMERA_NEAR 0.1f
#define CAMERA_FAR 1000

namespace JoyEngine
{
	Camera::Camera(IRenderManager* manager):
		m_manager(manager)
	{
	}

	void Camera::Enable()
	{
		m_manager->RegisterCamera(this);
		m_cameraUnit = CameraUnit(m_manager->GetAspect(),
		                          m_manager->GetWidth_f(),
		                          m_manager->GetHeight_f(),
		                          CAMERA_FOV,
		                          CAMERA_NEAR,
		                          CAMERA_FAR
		);
		m_enabled = true;
	}

	void Camera::Disable()
	{
		m_manager->UnregisterCamera(this);
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

	float Camera::GetFovRadians() const
	{
		return glm::radians(CAMERA_FOV);
	}

	float Camera::GetNear() const
	{
		return CAMERA_NEAR;
	}

	float Camera::GetFar() const
	{
		return CAMERA_FAR;
	}
}
