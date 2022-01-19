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
		float currentAngle = (JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_T) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_Y) ? Time::GetDeltaTime() : 0);
		glm::vec3 vec = glm::vec3(
#ifdef GLM_FORCE_LEFT_HANDED
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_D) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_A) ? Time::GetDeltaTime() : 0),
#else
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_A) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_D) ? Time::GetDeltaTime() : 0),
#endif
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_E) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_Q) ? Time::GetDeltaTime() : 0),
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_W) ? Time::GetDeltaTime() : 0) -
			(JoyContext::Input->GetKeyDown(KeyCode::KEYCODE_S) ? Time::GetDeltaTime() : 0));

		const glm::vec3 vecWorld = m_transform->GetModelMatrix()* glm::vec4(vec, 0);

		m_transform->SetPosition(
			m_transform->GetPosition() + vecWorld
		);
		m_transform->SetRotation(
			glm::angleAxis(currentAngle, glm::vec3(0, 1, 0))*
			m_transform->GetRotation()
		);
	}
}
