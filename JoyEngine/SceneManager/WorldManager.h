#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <memory>
#include "Scene.h"
#include "TreeStorage.h"
#include "Common/Singleton.h"
#include "RenderManager/IRenderer.h"
#include "RenderManager/Skybox.h"

namespace JoyEngine
{
	class WorldManager : public Singleton<WorldManager>
	{
	public:
		WorldManager() = delete;

		explicit WorldManager(HWND gameWindowHandle);

		void Init();

		void Start();

		template <typename... Args>
		GameObject* CreateGameObject(Args&&... args)
		{
			return m_sceneTree.Create<GameObject>(std::forward<Args>(args)...);
		}

		[[nodiscard]] TransformProvider& GetTransformProvider() const noexcept { return *m_transformProvider; }

		void Stop();

		void Update();

		~WorldManager();

	private:
		TreeStorage<GameObject, 512> m_sceneTree;
		Scene* m_scene = nullptr;

		std::unique_ptr<TransformProvider> m_transformProvider;
		std::unique_ptr<Skybox> m_skybox;
		std::unique_ptr<IRenderer> m_renderManager;
	};
}

#endif //SCENE_MANAGER_H
