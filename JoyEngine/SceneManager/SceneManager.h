#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <memory>
#include "Scene.h"
#include "TreeStorage.h"
#include "Common/Singleton.h"
#include "RenderManager/IRenderManager.h"

namespace JoyEngine
{
	class SceneManager : public Singleton<SceneManager>
	{
	public:
		SceneManager() = delete;

		SceneManager(const IRenderManager* rm):
			m_renderManager(rm),
			m_transformProvider(rm)
		{
		};

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

		~SceneManager();

	private:
		const IRenderManager* m_renderManager;
		TransformProvider m_transformProvider;
		TreeStorage<GameObject, 512> m_sceneTree;
		Scene* m_scene = nullptr;
	};
}

#endif //SCENE_MANAGER_H
