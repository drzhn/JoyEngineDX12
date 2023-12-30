#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <memory>
#include "Scene.h"
#include "TreeStorage.h"
#include "Common/Singleton.h"

namespace JoyEngine
{
	class WorldManager : public Singleton<WorldManager>
	{
	public:
		WorldManager() = default;

		void Init();

		void Start();

		template <typename... Args>
		GameObject* CreateGameObject(Args&&... args)
		{
			return m_sceneTree.Create<GameObject>(std::forward<Args>(args)...);
		}

		[[nodiscard]] TransformProvider& GetTransformProvider() noexcept { return m_transformProvider; }

		void Stop();

		void Update();

		~WorldManager();

	private:
		TransformProvider m_transformProvider;
		TreeStorage<GameObject, 512> m_sceneTree;
		Scene* m_scene = nullptr;
	};
}

#endif //SCENE_MANAGER_H
