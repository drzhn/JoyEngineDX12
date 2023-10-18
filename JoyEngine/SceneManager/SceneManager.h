#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <map>
#include <memory>


#include "Scene.h"
#include "Common/Singleton.h"


namespace JoyEngine
{
	class SceneManager : public Singleton<SceneManager>
	{
	public:
		SceneManager() = default;

		void Init();

		void Start();

		void Stop();

		void Update();

		~SceneManager();

	private:
		std::unique_ptr<Scene> m_scene = nullptr;
	};
}

#endif //SCENE_MANAGER_H
