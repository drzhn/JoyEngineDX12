#ifndef SCENE_H
#define SCENE_H

#include "GameObject.h"

namespace JoyEngine
{
	class Scene : public GameObject
	{
	public :
		Scene(const char* path);
		void Update();
	};
}

#endif //SCENE_H
