#include "Material.h"
#include <vector>


#include <rapidjson/document.h>

#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "RenderManager/RenderManager.h"

#include "SharedMaterial.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	Material::Material(GUID guid) : Resource(guid)
	{
		rapidjson::Document json = DataManager::Get()->GetSerializedData(guid, material);
		m_sharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial>(GUID::StringToGuid(json["sharedMaterial"].GetString()));

		std::map<std::string, std::string> bindings;
		for (auto& bindingJson : json["bindings"].GetArray())
		{
			bindings.insert({
				bindingJson["name"].GetString(),
				bindingJson["data"].GetString()
			});
		}

		InitMaterial(bindings, false);
	}

	Material::Material(
		GUID guid,
		ResourceHandle<SharedMaterial> sharedMaterial,
		const std::map<std::string, std::string>& bindings,
		bool bindingsArePaths = false)
		: Resource(guid), m_sharedMaterial(sharedMaterial)
	{
		InitMaterial(bindings, bindingsArePaths);
	}

	void Material::InitMaterial(const std::map<std::string, std::string>& bindings, bool bindingsArePaths)
	{
		for (const auto& binding : bindings)
		{
			const std::string& name = binding.first;
			const std::string& data = binding.second;

			ShaderInput const* shaderInput = m_sharedMaterial->GetShaderInputByName(name);

			if (shaderInput == nullptr) continue;

			switch (shaderInput->Type)
			{
			case D3D_SIT_CBUFFER:
				{
					break;
				}
			case D3D_SIT_TEXTURE:
				{
					if (!data.empty())
					{
						if (bindingsArePaths)
						{
							//Texture* t = ResourceManager::Get()->LoadResource<Texture>(GUID::Random(), data);
							m_textures.emplace_back(ResourceManager::Get()->LoadResource<Texture>(GUID::Random(), data));
						}
						else
						{
							m_textures.emplace_back(ResourceManager::Get()->LoadResource<Texture>(GUID::StringToGuid(data)));
						}
						m_rootParams.insert({
							m_sharedMaterial->GetRootIndexByName(name),
							m_textures.back()->GetSRV()
						});
					}
					break;
				}
			case D3D_SIT_SAMPLER:
				{
					ResourceView* samplerView = nullptr;
					switch (strHash(data.c_str()))
					{
					case strHash("texture"):
						samplerView = EngineSamplersProvider::GetLinearWrapSampler();
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
