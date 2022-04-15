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
		void SetMesh(GUID meshGuid, uint32_t vertexDataSize, uint32_t indexDataSize, std::ifstream& modelStream, uint32_t vertexDataStreamOffset, uint32_t indexDataStreamOffset);

		void SetMaterial(const std::string& materialName);
		void SetMaterial(const GUID& materialGuid);
		void SetMaterial(const ResourceHandle<Material>& mat);


		[[nodiscard]] Mesh* GetMesh() const noexcept;

		[[nodiscard]] Material* GetMaterial() const noexcept;

		[[nodiscard]] bool IsReady() const noexcept;

	private:
		ResourceHandle<Mesh> m_mesh;
		ResourceHandle<Material> m_material;
	};
}


#endif //MESHRENDERER_H
