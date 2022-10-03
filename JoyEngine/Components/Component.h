#ifndef COMPONENT_H
#define COMPONENT_H

#include "Common/Serializable.h"

namespace JoyEngine {

	class Transform;

	class Component : public Serializable {
	public:
		Component() = default;

		virtual ~Component() = default;

		virtual void Enable() = 0;

		virtual void Disable() = 0;

		virtual void Update() = 0;

		void SetTransform(Transform* t) { m_transform = t; }
		[[nodiscard]] Transform* GetTransform() const noexcept { return m_transform; }

		bool IsEnabled() const noexcept { return m_enabled; }

	protected:
		Transform* m_transform;

		bool m_enabled = false;
	};
}


#endif //COMPONENT_H
