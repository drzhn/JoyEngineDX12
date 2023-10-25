#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <vector>
#include <memory>

#include "RenderManager/TransformProvider.h"
#include "SceneManager/Transform.h"

#include "GameObject.h"
#include "TreeStorage.h"
#include "Components/Component.h"

namespace JoyEngine
{
	class Transform;
	class TransformProvider;

	class GameObject : public TreeEntry<GameObject>
	{
	public:
		explicit GameObject(const char* name, uint32_t transformIndex, TransformProvider& transformProvider) :
			m_name(name),
			m_transform(transformIndex, transformProvider)
		{
		}

		void AddChild(GameObject* child)
		{
			child->m_parent = this;
			child->m_nextSibling = this->m_firstChild;
			this->m_firstChild = child;

			child->GetTransform().UpdateMatrix();
		}

		[[nodiscard]] Transform& GetTransform() noexcept { return m_transform; }

		~GameObject();

		void Update();

		void AddComponent(std::unique_ptr<Component> component);

	protected:
		std::string m_name;
		Transform m_transform;

	private:
		std::vector<std::unique_ptr<Component>> m_components;
	};
}


#endif //GAME_OBJECT_H
