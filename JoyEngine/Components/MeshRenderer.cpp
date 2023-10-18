#include "MeshRenderer.h"


#include "ResourceManager/Material.h"
#include "ResourceManager/Mesh.h"

#include "EngineDataProvider/EngineDataProvider.h"

namespace JoyEngine
{
	MeshRenderer::MeshRenderer(GameObject& go, bool isStatic):
		Component(go),
		m_isStatic(isStatic)
	{
	}

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

	void MeshRenderer::SetMesh(const char* path)
	{
		m_mesh = ResourceManager::Get()->LoadResource<Mesh>(path);
	}


	void InitMesh(std::ifstream& modelStream)
	{
	}

	void MeshRenderer::SetMesh(
		uint32_t vertexDataSize,
		uint32_t indexDataSize,
		std::ifstream& modelStream,
		uint32_t vertexDataStreamOffset,
		uint32_t indexDataStreamOffset)
	{
		InitMesh(modelStream);
		m_mesh = ResourceManager::Get()->RegisterResource<Mesh>(
			new Mesh(
				vertexDataSize,
				indexDataSize,
				modelStream,
				vertexDataStreamOffset,
				indexDataStreamOffset
			));
	}

	//void MeshRenderer::SetMaterial(const std::string& materialName)
	//{
	//	m_material = EngineDataProvider::Get()->GetSampleMaterialByName(materialName); //for debug purposes 
	//}

	void MeshRenderer::SetMaterial(const char* path)
	{
		m_material = ResourceManager::Get()->LoadResource<Material>(path);
	}

	void MeshRenderer::SetMaterial(const ResourceHandle<Material>& mat)
	{
		m_material = mat;
	}

	bool MeshRenderer::IsStatic() const noexcept
	{
		return m_isStatic;
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
