#ifndef CAMERA_H
#define CAMERA_H

#include "Component.h"
#include <glm/glm.hpp>

namespace JoyEngine {
    class Camera : public Component {
    public:
        void Enable() override;
        void Disable() override;
        void Update() override;
        glm::mat4x4 GetProjMatrix() const;
        glm::mat4x4 GetViewMatrix() const;

    private:
        float m_aspect;
        float m_width;
        float m_height;
        float m_fov;
        float m_near;
        float m_far;
    };
}

#endif //CAMERA_H
