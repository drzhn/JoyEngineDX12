#include "MeshRenderer.h"

#include "JoyContext.h"

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
		//JoyContext::Resource->UnloadResource(m_material->GetGuid());
		//JoyContext::Resource->UnloadResource(m_mesh->GetGuid());
	}

	void MeshRenderer::SetMesh(GUID meshGuid)
	{
		m_mesh = JoyContext::Resource->LoadResource<Mesh>(meshGuid);
	}

	void MeshRenderer::SetMaterial(const std::string& materialName)
	{
		m_material = JoyContext::EngineMaterials->GetSampleMaterialByName(materialName);
	}

	void MeshRenderer::SetMaterial(const GUID& materialGuid)
	{
		m_material = JoyContext::Resource->LoadResource<Material>(materialGuid);
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
