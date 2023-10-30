#ifndef LIGHT_BEHAVIOUR_H
#define LIGHT_BEHAVIOUR_H

#include "Common/JoyObject.h"
#include "Components/Component.h"
#include "Common/Serialization.h"

namespace JoyEngine
{
	DECLARE_CLASS(LightBehaviour)

	class LightBehaviour : public Component
	{
		DECLARE_JOY_OBJECT(LightBehaviour, Component);

		REFLECT_FIELD(float, m_phase);
		REFLECT_FIELD(float, m_speed);
		REFLECT_FIELD(float, m_amplitude);

	public:
		explicit LightBehaviour(GameObject& go);

		void Enable() final;

		void Disable() final;

		void Update() final;
	};
}
#endif // LIGHT_BEHAVIOUR_H
