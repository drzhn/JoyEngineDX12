#include "Material.h"
#include <vector>
#include "JoyContext.h"

#include <rapidjson/document.h>

#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "ResourceManager/ResourceManager.h"
#include "RenderManager/RenderManager.h"

namespace JoyEngine
{
	Material::Material(GUID guid) : Resource(guid)
	{
		//rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, material);
	}

	Material::Material(GUID guid, MaterialData data) :
		Resource(guid)
	{
		m_sharedMaterial = data.sharedMaterial;
		m_rootParams = data.rootParams;
		m_heaps = data.heaps;
	}

	Material::~Material()
	{
	}

	SharedMaterial* Material::GetSharedMaterial() const noexcept
	{
		return m_sharedMaterial;
	}

	bool Material::IsLoaded() const noexcept
	{
		return true;
	}
}
