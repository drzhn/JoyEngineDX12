#include "RoomBehaviour.h"


#include "InputManager/InputManager.h"

#include "Common/Time.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"

namespace JoyEngine
{
	RoomBehaviour::RoomBehaviour(GameObject& go)
		: Component(go)
	{
	}

	void RoomBehaviour::Enable()
	{
		m_enabled = true;
	}

	void RoomBehaviour::Disable()
	{
		m_enabled = false;
	}

	void RoomBehaviour::Update()
	{
		//if (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_A))
		//{
		//	m_transform->SetPosition(
		//		m_transform->GetPosition() + glm::vec3(Time::GetDeltaTime(), 0, 0)
		//	);
		//}
		//if (InputManager::Get()->GetKeyDown(KeyCode::KEYCODE_D))
		//{
		//	m_transform->SetPosition(
		//		m_transform->GetPosition() + glm::vec3(-Time::GetDeltaTime(), 0, 0)
		//	);
		//}
		m_gameObject.GetTransform()->SetRotation( // glm::angleAxis(glm::pi<float>() / 2, glm::vec3(-1, 0, 0)));// *
			jmath::angleAxis(jmath::xup, Time::GetTime() * m_speed));
	}
}
