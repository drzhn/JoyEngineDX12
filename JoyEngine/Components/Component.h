#ifndef COMPONENT_H
#define COMPONENT_H

#include "Common/Serializable.h"

namespace JoyEngine
{
	class GameObject;

	class Component : public Serializable
	{
	public:
		Component() = delete;

		explicit Component(GameObject& go):
			m_gameObject(go)
		{
		}

		virtual ~Component() = default;

		virtual void Enable() = 0;

		virtual void Disable() = 0;

		virtual void Update() = 0;

		[[nodiscard]] GameObject& GetGameObject() const noexcept { return m_gameObject; }
		[[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

	protected:
		GameObject& m_gameObject;

		bool m_enabled = false;
	};
}


#endif //COMPONENT_H
