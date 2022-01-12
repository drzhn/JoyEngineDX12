#ifndef CAMERA_BEHAVIOUR_H
#define CAMERA_BEHAVIOUR_H

#include "Components/Component.h"
#include "Common/Serialization.h"

namespace JoyEngine
{
	class CameraBehaviour : public Component
	{
		DECLARE_CLASS_NAME(CameraBehaviour)

		REFLECT_FIELD(float, m_speed);

	public :
		CameraBehaviour() = default;

		void Enable() final;

		void Disable() final;

		void Update() final;
	};
}

#endif //CAMERA_BEHAVIOUR_H
