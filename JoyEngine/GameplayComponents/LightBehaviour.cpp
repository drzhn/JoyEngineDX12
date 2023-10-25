#include "LightBehaviour.h"

#include <cmath>

#include "Common/Time.h"
#include "SceneManager/GameObject.h"


namespace JoyEngine
{
	LightBehaviour::LightBehaviour(GameObject& go)
		:Component(go)
	{

	}

	void LightBehaviour::Enable()
	{
	}

	void LightBehaviour::Disable()
	{
	}

	void LightBehaviour::Update()
	{
		jmath::vec3 position = GetGameObject().GetTransform().GetPosition();
		position.x = sin(Time::GetTime() * m_speed + m_phase) * m_amplitude;
		GetGameObject().GetTransform().SetPosition(position);
	}
}
