#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <optional>

#include "Component.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GUID.h"
#include "Utils/Assert.h"
//#include "ResourceManager/Material.h"

namespace JoyEngine
{
	class Material;
	class Mesh;

	class MeshRenderer : public Component
	{
	public:
		MeshRenderer() = default;

		void Enable() final;

		void Disable() final;

		void Update() final
		{
		}

		~MeshRenderer() override;

		void SetMesh(GUID meshGuid);

		void SetMaterial(const std::string& materialName);
		void SetMaterial(const GUID& materialGuid);


		[[nodiscard]] Mesh* GetMesh() const noexcept;

		[[nodiscard]] Material* GetMaterial() const noexcept;

		[[nodiscard]] bool IsReady() const noexcept;

	private:
		ResourceHandle<Mesh> m_mesh;
		ResourceHandle<Material> m_material;
	};
}


#endif //MESHRENDERER_H
