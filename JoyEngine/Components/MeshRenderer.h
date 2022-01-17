#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <optional>

#include "Component.h"
#include "ResourceManager/Material.h"
#include "Utils/GUID.h"
#include "Utils/Assert.h"
#include "ResourceManager/Mesh.h"
//#include "ResourceManager/Material.h"

namespace JoyEngine {
    //class Material;

    class MeshRenderer : public Component {
    public:
        MeshRenderer() = default;

        void Enable() final;

        void Disable() final;

        void Update() final {};

        ~MeshRenderer() override;

        void SetMesh(GUID meshGuid);

        void SetMaterial(GUID materialGuid);


        [[nodiscard]]Mesh *GetMesh() const noexcept;

        [[nodiscard]]Material *GetMaterial() const noexcept;

        [[nodiscard]] bool IsReady() const noexcept;

    private:
        Mesh* m_mesh;
        Material* m_material;
    };
}


#endif //MESHRENDERER_H
