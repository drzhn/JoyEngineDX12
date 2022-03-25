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
		rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, material);
		m_sharedMaterial = GUID::StringToGuid(json["sharedMaterial"].GetString());

		for (auto& bindingJson : json["bindings"].GetArray())
		{
			std::string name = bindingJson["name"].GetString();
			std::string data = bindingJson["data"].GetString();

			const ShaderInput& shaderInput = m_sharedMaterial->GetShaderInputByName(name);

			switch (shaderInput.Type)
			{
			case D3D_SIT_CBUFFER:
				{
					break;
				}
			case D3D_SIT_TEXTURE:
				{
					m_textures.emplace_back(GUID::StringToGuid(data));
					m_rootParams.insert({
						m_sharedMaterial->GetRootIndexByName(name),
						m_textures.back()->GetResourceView()
					});
					break;
				}
			case D3D_SIT_SAMPLER:
				{
					ResourceView* samplerView = nullptr;
					switch (strHash(data.c_str()))
					{
					case strHash("texture"):
						samplerView = Texture::GetTextureSampler();
						break;
					default:
						ASSERT(false);
						break;
					}

					m_rootParams.insert({
						m_sharedMaterial->GetRootIndexByName(name),
						samplerView
					});
					break;
				}

			case D3D_SIT_UAV_RWTYPED:
			case D3D_SIT_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			case D3D_SIT_UAV_FEEDBACKTEXTURE:

			case D3D_SIT_TBUFFER:
			case D3D_SIT_BYTEADDRESS:
			case D3D_SIT_RTACCELERATIONSTRUCTURE:
			default:
				ASSERT(false);
			}
		}
	}

	//Material::Material(GUID guid, MaterialArgs data) :
	//	Resource(guid),
	//	m_sharedMaterial(data.sharedMaterial),
	//	m_rootParams(data.rootParams),
	//	m_textures(data.textures),
	//	m_buffers(data.buffers)
	//{
	//	//for (const auto& rp : data.rootParams)
	//	//{
	//	//	m_heaps.push_back(rp.second);
	//	//}
	//}

	SharedMaterial* Material::GetSharedMaterial() const noexcept
	{
		return m_sharedMaterial;
	}

	bool Material::IsLoaded() const noexcept
	{
		return true;
	}
}
