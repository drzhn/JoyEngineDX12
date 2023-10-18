#include "Material.h"

#include <vector>

#include <rapidjson/document.h>

#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "GraphicsManager/GraphicsManager.h"
#include "DataManager/DataManager.h"
#include "RenderManager/RenderManager.h"

#include "SharedMaterial.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	uint32_t Material::s_currentMaterialIndex = 0;

	Material::Material(const char* materialPath) :
		Resource(materialPath),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineDataProvider::Get()->GetGBufferWriteSharedMaterial()) // We will only use standard material for serialized materials
	{
		rapidjson::Document json = DataManager::Get()->GetSerializedData(materialPath, material);

		std::map<std::string, std::string> bindings;
		for (auto& bindingJson : json["bindings"].GetArray())
		{
			bindings.insert({
				bindingJson["name"].GetString(),
				bindingJson["data"].GetString()
			});
		}

		InitMaterial(bindings);
	}

	Material::Material(uint64_t id, const std::map<std::string, std::string>& bindings) :
		Resource(id),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineDataProvider::Get()->GetGBufferWriteSharedMaterial())
	{
		InitMaterial(bindings);
	}

	void Material::InitMaterial(const std::map<std::string, std::string>& bindings)
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
					ResourceView* srv = EngineDataProvider::Get()->GetNullTextureView();
					if (!data.empty())
					{
						m_textures.emplace_back(ResourceManager::Get()->LoadResource<Texture>(data.c_str()));
						srv = m_textures.back()->GetSRV();
					}

					m_rootParams.insert({
						m_sharedMaterial->GetGraphicsPipeline()->GetBindingIndexByName(name),
						srv
					});
					EngineDataProvider::Get()->SetMaterialData(m_materialIndex, srv->GetDescriptorIndex());

					break;
				}
			case D3D_SIT_SAMPLER:
				{
					ResourceView* samplerView = nullptr;
					switch (StrHash32(data.c_str()))
					{
					case StrHash32("linearWrap"):
						samplerView = EngineSamplersProvider::GetLinearWrapSampler();
						break;
					case StrHash32("linearClamp"):
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
