#include "MeshRenderer.h"



#include "ResourceManager/Material.h"
#include "ResourceManager/Mesh.h"

#include "EngineMaterialProvider/EngineMaterialProvider.h"

namespace JoyEngine
{
	void MeshRenderer::Enable()
	{
		//ASSERT(m_mesh != nullptr && m_material != nullptr);
		m_material->GetSharedMaterial()->RegisterMeshRenderer(this);
		m_enabled = true;
	}

	void MeshRenderer::Disable()
	{
		m_material->GetSharedMaterial()->UnregisterMeshRenderer(this);
		m_enabled = false;
	}

	MeshRenderer::~MeshRenderer()
	{
		if (m_enabled)
		{
			Disable();
		}
	}

	void MeshRenderer::SetMesh(GUID meshGuid)
	{
		m_mesh = ResourceManager::Get()->LoadResource<Mesh>(meshGuid);
	}


	void InitMesh(std::ifstream& modelStream)
	{
	}

	void MeshRenderer::SetMesh(
		GUID meshGuid,
		uint32_t vertexDataSize,
		uint32_t indexDataSize,
		std::ifstream& modelStream,
		uint32_t vertexDataStreamOffset,
		uint32_t indexDataStreamOffset)
	{
		InitMesh(modelStream);
		m_mesh = ResourceManager::Get()->LoadResource<Mesh>(
			meshGuid,
			vertexDataSize,
			indexDataSize,
			modelStream,
			vertexDataStreamOffset,
			indexDataStreamOffset
		);
	}

	void MeshRenderer::SetMaterial(const std::string& materialName)
	{
		m_material = EngineMaterialProvider::Get()->GetSampleMaterialByName(materialName); //for debug purposes 
	}

	void MeshRenderer::SetMaterial(const GUID& materialGuid)
	{
		m_material = ResourceManager::Get()->LoadResource<Material>(materialGuid);
	}

	void MeshRenderer::SetMaterial(const ResourceHandle<Material>& mat)
	{
		m_material = mat;
	}

	Mesh* MeshRenderer::GetMesh() const noexcept
	{
		return m_mesh;
	}

	Material* MeshRenderer::GetMaterial() const noexcept
	{
		return m_material;
	}

	bool MeshRenderer::IsReady() const noexcept
	{
		return m_mesh->IsLoaded(); // && m_material->IsLoaded();
	}
}
