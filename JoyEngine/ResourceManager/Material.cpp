#include "Material.h"

#include <vector>

#include "Common/HashDefs.h"
#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"

#include "SharedMaterial.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	uint32_t Material::s_currentMaterialIndex = 0;

	std::map<std::string, size_t> Material::s_fieldOffsets = {
		{"DiffuseMap", offsetof(StandardMaterial, DiffuseMap)},
		{"EmissiveMap", offsetof(StandardMaterial, EmissiveMap)},
		{"EmissiveFactor", offsetof(StandardMaterial, EmissiveFactor)},
		{"AmbientMap", offsetof(StandardMaterial, AmbientMap)},
		{"AmbientFactor", offsetof(StandardMaterial, AmbientFactor)},
		{"NormalMap", offsetof(StandardMaterial, NormalMap)},
		{"TransparentColor", offsetof(StandardMaterial, TransparentColor)},
		{"TransparencyFactor", offsetof(StandardMaterial, TransparencyFactor)},
		{"SpecularMap", offsetof(StandardMaterial, SpecularMap)},
		{"SpecularFactor", offsetof(StandardMaterial, SpecularFactor)},
		{"ReflectionMap", offsetof(StandardMaterial, ReflectionMap)},
		{"ReflectionFactor", offsetof(StandardMaterial, ReflectionFactor)},
		{"ShininessMap", offsetof(StandardMaterial, ShininessMap)},
	};

	Material::Material(const char* materialPath) :
		Resource(materialPath),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineDataProvider::Get()->GetStandardPhongSharedMaterial()) // We will only use standard Material for serialized materials
	{
		rapidjson::Document json = DataManager::Get()->GetSerializedData(materialPath, AssetType::Material);

		InitMaterial(json);
	}

	Material::Material(uint64_t id, rapidjson::Value& materialJson) :
		Resource(id),
		m_materialIndex(s_currentMaterialIndex++),
		m_sharedMaterial(EngineDataProvider::Get()->GetStandardPhongSharedMaterial())
	{
		InitMaterial(materialJson);
	}

	void Material::InitMaterial(rapidjson::Value& materialJson)
	{
		const auto& bindingJson = materialJson["bindings"];
		for (auto member = bindingJson.MemberBegin(); member != bindingJson.MemberEnd(); ++member)
		{
			const std::string& name = member->name.GetString();

			if (member->value.IsString())
			{
				const std::string& data = member->value.GetString();

				ShaderInput const* shaderInput = m_sharedMaterial->GetGraphicsPipeline()->GetShaderInputByName(name);

				if (shaderInput == nullptr) continue;

				switch (shaderInput->Type)
				{
				case D3D_SIT_CBUFFER:
					{
						ASSERT_DESC(false, "JoyEngine doesn't support buffers in serialized materials yet");
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
						uint32_t descriptorIndex = srv->GetDescriptorIndex();

						EngineDataProvider::Get()->SetMaterialData(
							m_materialIndex,
							s_fieldOffsets.at(name), &descriptorIndex, sizeof(uint32_t));

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
			else
			{
				if (member->value.IsFloat())
				{
					float data = member->value.GetFloat();
					EngineDataProvider::Get()->SetMaterialData(
						m_materialIndex,
						s_fieldOffsets.at(name),
						&data,
						sizeof(float));
				}
				else
				{
					ASSERT(false);
				}
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
