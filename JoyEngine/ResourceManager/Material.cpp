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

#include "SharedMaterial.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	Material::Material(GUID guid) : Resource(guid)
	{
		//rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, material);
	}

	Material::Material(GUID guid, MaterialArgs data) :
		Resource(guid),
		m_sharedMaterial(data.sharedMaterial),
		m_rootParams(data.rootParams),
		m_textures(data.textures),
		m_buffers(data.buffers)
	{
		//for (const auto& rp : data.rootParams)
		//{
		//	m_heaps.push_back(rp.second);
		//}
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
