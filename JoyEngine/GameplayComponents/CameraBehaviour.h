#ifndef CAMERA_BEHAVIOUR_H
#define CAMERA_BEHAVIOUR_H

#include "Common/JoyObject.h"
#include "Components/Component.h"
#include "Common/Serialization.h"

namespace JoyEngine
{
	DECLARE_CLASS(CameraBehaviour)

	class CameraBehaviour : public Component
	{
		DECLARE_JOY_OBJECT(CameraBehaviour, Component);

		REFLECT_FIELD(float, m_speed);
	public :
		explicit CameraBehaviour(GameObject& go);

		void Enable() final;

		void Disable() final;

		void Update() final;
	};
}

#endif //CAMERA_BEHAVIOUR_H
