#include "Camera.h"

#include "RenderManager/IRenderer.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

namespace JoyEngine
{
	Camera::Camera(GameObject& go, IRenderer* manager, float cameraNear, float cameraFar, float cameraFov) :
		Component(go),
		m_cameraUnit(CameraUnit(manager->GetAspect(),
		                        manager->GetWidth_f(),
		                        manager->GetHeight_f(),
		                        cameraFov,
		                        cameraNear,
		                        cameraFar

		)),
		m_manager(manager)
	{
	}

	void Camera::Enable()
	{
		m_manager->RegisterCamera(this);
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

	jmath::mat4x4 Camera::GetViewMatrix() const
	{
		return m_cameraUnit.GetViewMatrix(
			m_gameObject.GetTransform().GetXPosition(),
			m_gameObject.GetTransform().GetRotation()
		);
	}

	jmath::mat4x4 Camera::GetProjMatrix() const
	{
		return m_cameraUnit.GetProjMatrix();
	}

	float Camera::GetFovRadians() const
	{
		return m_cameraUnit.GetFOVRadians();
	}

	float Camera::GetNear() const
	{
		return m_cameraUnit.GetNear();
	}

	float Camera::GetFar() const
	{
		return m_cameraUnit.GetFar();
	}

	float Camera::GetAspect() const
	{
		return m_cameraUnit.GetAspect();
	}
}
