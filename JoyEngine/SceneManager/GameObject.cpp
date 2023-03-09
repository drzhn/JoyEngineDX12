#include "GameObject.h"

namespace JoyEngine {
    
    void GameObject::Update()
    {
        for (const auto& c : m_components)
        {
            c->Update();
        }
    }

    void GameObject::AddComponent(std::unique_ptr<Component> component) {
        component->Enable();
        m_components.push_back(std::move(component));
    }
}
