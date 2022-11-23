#include "Material.h"

#include <vector>

#include <rapidjson/document.h>

#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "RenderManager/RenderManager.h"

#include "SharedMaterial.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	uint32_t Material::s_currentMaterialIndex = 0;

	Material::Material(GUID guid) :
		Resource(guid),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineMaterialProvider::Get()->GetGBufferWriteSharedMaterial()) // We will only use standard material for serialized materials
	{
		rapidjson::Document json = DataManager::Get()->GetSerializedData(guid, material);

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

	Material::Material(GUID guid, const std::map<std::string, std::string>& bindings, bool bindingsArePaths = false) :
		Resource(guid),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineMaterialProvider::Get()->GetGBufferWriteSharedMaterial())
	{
		InitMaterial(bindings, bindingsArePaths);
	}

	void Material::InitMaterial(const std::map<std::string, std::string>& bindings, bool bindingsArePaths)
	{
		for (const auto& binding : bindings)
		{
			const std::string& name = binding.first;
			const std::string& data = binding.second;

			ShaderInput const* shaderInput = m_sharedMaterial->GetGraphicsPipeline()->GetShaderInputByName(name);

			if (shaderInput == nullptr) continue;

			switch (shaderInput->Type)
			{
			case D3D_SIT_CBUFFER:
				{
					break;
				}
			case D3D_SIT_TEXTURE:
				{
					ResourceView* srv = EngineMaterialProvider::Get()->GetNullTextureView();
					if (!data.empty())
					{
						if (bindingsArePaths)
						{
							m_textures.emplace_back(ResourceManager::Get()->LoadResource<Texture>(GUID::Random(), data));
						}
						else
						{
							m_textures.emplace_back(ResourceManager::Get()->LoadResource<Texture>(GUID::StringToGuid(data)));
						}
						srv = m_textures.back()->GetSRV();
					}

					m_rootParams.insert({
						m_sharedMaterial->GetGraphicsPipeline()->GetBindingIndexByName(name),
						srv
					});
					EngineMaterialProvider::Get()->SetMaterialData(m_materialIndex, srv->GetDescriptorIndex());

					break;
				}
			case D3D_SIT_SAMPLER:
				{
					ResourceView* samplerView = nullptr;
					switch (strHash(data.c_str()))
					{
					case strHash("linearWrap"):
						samplerView = EngineSamplersProvider::GetLinearWrapSampler();
						break;
					case strHash("linearClamp"):
						samplerView = EngineSamplersProvider::GetLinearClampSampler();
						break;
					default:
						ASSERT(false);
						break;
					}

					m_rootParams.insert({
						m_sharedMaterial->GetGraphicsPipeline()->GetBindingIndexByName(name),
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

	SharedMaterial* Material::GetSharedMaterial() const noexcept
	{
		return m_sharedMaterial;
	}

	bool Material::IsLoaded() const noexcept
	{
		return true;
	}
}
