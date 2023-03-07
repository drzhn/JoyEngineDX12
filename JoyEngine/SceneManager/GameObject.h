#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>
#include <vector>
#include <memory>

#include "Transform.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"

namespace JoyEngine
{
	class GameObject
	{
	public:
		explicit GameObject() : GameObject("GameObject")
		{
		}

		explicit GameObject(const char* name):
			m_transform(Transform()),
			m_name(name)
		{
		}

		~GameObject()
		{
			for (const auto& component : m_components)
			{
				component->Disable();
			}
		}

		void Update();

		Transform* GetTransform() { return &m_transform; }

		void AddComponent(std::unique_ptr<Component> component);

	private:
		Transform m_transform;
		std::string m_name;
		std::vector<std::unique_ptr<Component>> m_components;
	};
}


#endif //GAME_OBJECT_H
