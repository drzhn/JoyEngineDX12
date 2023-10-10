#include "CameraBehaviour.h"

#include "Common/Time.h"
#include "InputManager/InputManager.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

namespace JoyEngine
{
	CameraBehaviour::CameraBehaviour(GameObject& go)
		: Component(go)
	{
	}

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
#ifdef jmath_FORCE_LEFT_HANDED
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


		jmath::vec3 vec = jmath::vec3(deltaX, deltaY, deltaZ);

		const jmath::xvec4 vecWorld = jmath::rotate3(
			jmath::loadVec4(jmath::vec4(deltaX, deltaY, deltaZ, 1)),
			m_gameObject.GetTransform()->GetRotation()
		);

		m_gameObject.GetTransform()->SetXPosition(
			m_gameObject.GetTransform()->GetXPosition() + jmath::mul(m_speed, vecWorld)
		);

		m_gameObject.GetTransform()->SetRotation(
			jmath::mul(m_gameObject.GetTransform()->GetRotation(),
			           jmath::mul(jmath::angleAxis(m_gameObject.GetTransform()->GetXRight(), deltaXRotation),
			                      jmath::angleAxis(jmath::xup, deltaYRotation)))
		);
	}
}
