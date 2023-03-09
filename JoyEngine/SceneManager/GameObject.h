#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <string>
#include <vector>
#include <memory>

#include "Transform.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"
#include "RenderManager/TransformProvider.h"

namespace JoyEngine
{
	class GameObject
	{
	public:
		explicit GameObject(const char* name, uint32_t transformIndex, TransformProvider* provider) :
			m_transformIndex(transformIndex),
			m_provider(provider),
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

		[[nodiscard]] Transform* GetTransform() const noexcept { return &m_provider->GetTransform(m_transformIndex); }
		[[nodiscard]] uint32_t GetTransformIndex() const noexcept { return m_transformIndex; }
		[[nodiscard]] uint32_t const* GetTransformIndexPtr() const noexcept { return &m_transformIndex; }
		void AddComponent(std::unique_ptr<Component> component);

	private:
		const uint32_t m_transformIndex;
		TransformProvider* m_provider;

		std::string m_name;
		std::vector<std::unique_ptr<Component>> m_components;
	};
}


#endif //GAME_OBJECT_H
