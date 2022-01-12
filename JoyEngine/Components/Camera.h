#ifndef CAMERA_H
#define CAMERA_H

#include "Component.h"
#include <glm/glm.hpp>

namespace JoyEngine {
    class Camera : public Component {
    public:
        virtual void Enable() override;
        virtual void Disable() override;
        virtual void Update() override;
        glm::mat4x4 GetProjMatrix();
        glm::mat4x4 GetViewMatrix();

    private:
        float m_aspect;
        float m_fov;
        float m_near;
        float m_far;
    };
}

#endif //CAMERA_H
