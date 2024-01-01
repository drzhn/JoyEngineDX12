#ifndef SCENE_H
#define SCENE_H

#include <rapidjson/document.h>

#include "GameObject.h"

namespace JoyEngine
{
	class Scene : public GameObject
	{
	public :
		Scene() = delete;

		explicit Scene(rapidjson::Value& json);
		void Update();
	};
}

#endif //SCENE_H
