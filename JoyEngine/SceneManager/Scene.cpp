#include "Scene.h"

#include "rapidjson/document.h"


#include "Common/Serialization.h"
#include "Common/SerializationUtils.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "DataManager/DataManager.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/MtlBinaryParser.h"

namespace JoyEngine
{
	inline void ParseTransform(const std::unique_ptr<GameObject>& go, const rapidjson::Value& transformValue)
	{
		glm::vec3 vec;
		SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localPosition"], &vec, 1);
		go->GetTransform()->SetPosition(vec);
		SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localRotation"], &vec, 1);
		go->GetTransform()->SetRotation(vec);
		SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localScale"], &vec, 1);
		go->GetTransform()->SetScale(vec);
	}

	inline void ParseColor(float color[4], const rapidjson::Value& colorValue)
	{
		ASSERT(colorValue.IsArray());

		auto arr = colorValue.GetArray();
		ASSERT(arr.Size() == 4);

		for (int i = 0; i < 4; i++)
		{
			auto& obj = arr[i];
			ASSERT(obj.IsFloat());
			color[i] = obj.GetFloat();
		}
	}

	Scene::Scene(const GUID& guid)
	{
		rapidjson::Document json = DataManager::Get()->GetSerializedData(guid, scene);
		m_name = json["name"].GetString();

		rapidjson::Value& val = json["objects"];
		for (auto& obj : val.GetArray())
		{
			std::string objType = obj["type"].GetString();

			if (objType == "obj_simple")
			{
				std::unique_ptr<GameObject> go = std::make_unique<GameObject>(
					obj["name"].GetString(),
					RenderManager::Get()->GetTransformProvider()->Allocate(),
					RenderManager::Get()->GetTransformProvider()
				);

				rapidjson::Value& transformValue = obj["transform"];
				ParseTransform(go, transformValue);

				for (auto& component : obj["components"].GetArray())
				{
					std::string type = std::string(component["type"].GetString());
					if (type == "renderer")
					{
						bool isStatic = component["static"].GetBool();
						std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>(*go, isStatic);
						mr->SetMesh(GUID::StringToGuid(component["model"].GetString()));
						mr->SetMaterial(GUID::StringToGuid(component["material"].GetString()));
						go->AddComponent(std::move(mr));
					}
					else if (type == "component")
					{
						ASSERT(component.HasMember("component"));
						auto type = component["component"].GetString();
						ASSERT(SerializableClassFactory::GetInstance() != nullptr);
						std::unique_ptr<Serializable> s = SerializableClassFactory::GetInstance()->Deserialize(
							*go, component["fields"], component["component"].GetString());
						auto* c_ptr = dynamic_cast<Component*>(s.release());
						ASSERT(c_ptr != nullptr);
						std::unique_ptr<Component> c(c_ptr);
						go->AddComponent(std::move(c));
					}
					else if (type == "camera")
					{
						ASSERT(component.HasMember("near"));
						const float cameraNear = component["near"].GetFloat();
						ASSERT(component.HasMember("far"));
						const float cameraFar = component["far"].GetFloat();
						ASSERT(component.HasMember("fov"));
						const float cameraFov = component["fov"].GetFloat();

						go->AddComponent(std::make_unique<Camera>(*go, RenderManager::Get(), cameraNear, cameraFar, cameraFov));
					}
					else if (type == "light")
					{
						std::string lightTypeStr = std::string(component["lightType"].GetString());

						if (lightTypeStr == "point")
						{
							float intensity = component["intensity"].GetFloat();
							float radius = component["radius"].GetFloat();
							float color[4];
							ParseColor(color, component["color"]);

							std::unique_ptr<PointLight> light = std::make_unique<PointLight>(
								*go,
								RenderManager::Get()->GetLightSystem(),
								radius,
								intensity,
								color);
							go->AddComponent(std::move(light));
						}
						//else if (lightTypeStr == "capsule")
						//{
						//	float intensity = component["intensity"].GetFloat();
						//	float radius = component["radius"].GetFloat();
						//	float height = component["height"].GetFloat();

						//	light = std::make_unique<Light>(LightType::Capsule, intensity, radius, height, 0.0f, 0.0f);
						//}
						//else if (lightTypeStr == "spot")
						//{
						//	float intensity = component["intensity"].GetFloat();
						//	float angle = component["angle"].GetFloat();
						//	float height = component["height"].GetFloat();

						//	light = std::make_unique<Light>(LightType::Spot, intensity, 0.0f, height, angle, 0.0f);
						//}
						//else 
						else if (lightTypeStr == "direction")
						{
							float intensity = component["intensity"].GetFloat();
							float ambient = component["ambient"].GetFloat();
							float color[4];
							ParseColor(color, component["color"]);

							std::unique_ptr<DirectionalLight> light = std::make_unique<DirectionalLight>(
								*go,
								RenderManager::Get()->GetLightSystem(),
								intensity,
								ambient,
								color);
							go->AddComponent(std::move(light));
						}
						else
						{
							ASSERT(false);
						}
					}
					//else if (type == "particle_system")
					//{
					//	auto ps = std::make_unique<ParticleSystem>();
					//	go->AddComponent(std::move(ps));
					//}
					//else if (type == "cubemap_renderer")
					//{
					//	auto cr = std::make_unique<CubemapRenderer>();
					//	go->AddComponent(std::move(cr));
					//}
				}
				m_objects.push_back(std::move(go));
			}
			if (objType == "obj_complex")
			{
				std::string nameStr = obj["name"].GetString();
				GUID modelGuid = GUID::StringToGuid(obj["model"].GetString());
				GUID materialGuid = GUID::StringToGuid(obj["material"].GetString());
				bool isStatic = obj["static"].GetBool();
				std::unique_ptr<MtlBinaryParser> parser = std::make_unique<MtlBinaryParser>(modelGuid, materialGuid);
				MtlMeshStreamData* data = parser->Next();
				int objectIndex = 0;
				while (data != nullptr)
				{
					std::unique_ptr<GameObject> go = std::make_unique<GameObject>(
						obj["name"].GetString() + objectIndex,
						RenderManager::Get()->GetTransformProvider()->Allocate(),
						RenderManager::Get()->GetTransformProvider()
					);

					rapidjson::Value& transformValue = obj["transform"];
					ParseTransform(go, transformValue);

					std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>(*go, isStatic);
					mr->SetMesh(GUID::Random(),
					            data->vertexDataSize,
					            data->indexDataSize,
					            parser->GetModelStream(),
					            data->vertexStreamOffset,
					            data->indexStreamOffset);
					mr->SetMaterial(parser->GetMaterialByIndex(data->materialIndex));
					go->AddComponent(std::move(mr));

					m_objects.push_back(std::move(go));

					data = parser->Next();
					objectIndex++;
				}
			}
		}
	}

	void Scene::Update()
	{
		for (const auto& object : m_objects)
		{
			object->Update();
		}
	}
}
