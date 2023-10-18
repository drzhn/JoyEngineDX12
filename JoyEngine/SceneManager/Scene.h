#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <memory>

#include "GameObject.h"


namespace JoyEngine {
    class Scene {
    public :
        Scene(const char* path);

        void Update();
    private:
        std::string m_name;
        std::vector<std::unique_ptr<GameObject>> m_objects;
    };
}

#endif //SCENE_H
