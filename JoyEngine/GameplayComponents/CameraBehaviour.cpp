#include "CameraBehaviour.h"

#include <glm/ext/quaternion_trigonometric.hpp>


#include "Common/Time.h"
#include "InputManager/InputManager.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

namespace JoyEngine
{
	DECLARE_CLASS(CameraBehaviour)

	void CameraBehaviour::Enable()
	{
		m_enabled = true;
	}

	void CameraBehaviour::Disable()
	{
		m_enabled = false;
	}

	void CameraBehaviour::Update()
	{
		float deltaTime = Time::GetDeltaTime();

		float deltaX =
#ifdef GLM_FORCE_LEFT_HANDED
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_D) ? deltaTime : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_A) ? deltaTime : 0);
#else
		(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_D) ? deltaTime : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_A) ? deltaTime : 0);
#endif

		float deltaZ = (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_W) ? Time::GetDeltaTime() : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_S) ? Time::GetDeltaTime() : 0);

		float deltaY = (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_K) ? Time::GetDeltaTime() : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_M) ? Time::GetDeltaTime() : 0);

		float deltaYRotation = (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_E) ? Time::GetDeltaTime() : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_Q) ? Time::GetDeltaTime() : 0);

		float deltaXRotation = (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_N) ? Time::GetDeltaTime() : 0) -
			(InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_J) ? Time::GetDeltaTime() : 0);


		glm::vec3 vec = glm::vec3(deltaX, deltaY, deltaZ);

		const glm::vec3 vecWorld = m_gameObject.GetTransform()->GetRotation() * glm::vec4(deltaX, deltaY, deltaZ, 1);

		m_gameObject.GetTransform()->SetPosition(
			m_gameObject.GetTransform()->GetPosition() + vecWorld * m_speed
		);
		m_gameObject.GetTransform()->SetRotation(
			glm::angleAxis(deltaYRotation, glm::vec3(0, 1, 0)) *
			glm::angleAxis(deltaXRotation, m_gameObject.GetTransform()->GetRight()) *
			m_gameObject.GetTransform()->GetRotation()
		);
	}
}
