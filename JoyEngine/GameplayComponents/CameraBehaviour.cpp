#include "CameraBehaviour.h"

#include "JoyContext.h"
#include "Common/Time.h"
#include "InputManager/InputManager.h"

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
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_D) ? deltaTime : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_A) ? deltaTime : 0);
#else
		(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_A) ? deltaTime : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_D) ? deltaTime : 0);
#endif

		float deltaZ = (JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_W) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_S) ? Time::GetDeltaTime() : 0);

		float deltaY = (JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_K) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_M) ? Time::GetDeltaTime() : 0);

		float deltaAngle = (JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_E) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_Q) ? Time::GetDeltaTime() : 0);


		glm::vec3 vec = glm::vec3(deltaX, deltaY, deltaZ);
		
		const glm::vec3 vecWorld = m_transform->GetRotation() * glm::vec4(deltaX,deltaY,deltaZ, 1);

		m_transform->SetPosition(
			m_transform->GetPosition() + vecWorld
		);
		m_transform->SetRotation(
			glm::angleAxis(deltaAngle, glm::vec3(0, 1, 0)) *
			m_transform->GetRotation()
		);
	}
}
