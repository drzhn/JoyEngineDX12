#include "Scene.h"

#include "WorldManager.h"
#include "rapidjson/document.h"


#include "Common/Serialization.h"
#include "Common/SerializationUtils.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "DataManager/DataManager.h"
#include "RenderManager/RenderManager.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	inline void ParseTransform(GameObject* go, const rapidjson::Value& transformValue)
	{
		jmath::vec3 vec;
		SerializationUtils::DeserializeToPtr(StrHash32("vec3"), transformValue["localPosition"], &vec, 1);
		go->GetTransform().SetPosition(vec);
		SerializationUtils::DeserializeToPtr(StrHash32("vec3"), transformValue["localRotation"], &vec, 1);
		go->GetTransform().SetRotation(vec);
		SerializationUtils::DeserializeToPtr(StrHash32("vec3"), transformValue["localScale"], &vec, 1);
		go->GetTransform().SetScale(vec);
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

	GameObject* ParseGameObjectJson(rapidjson::Value& obj, GameObject* parent)
	{
		std::string objType = obj["asset_type"].GetString();

		if (objType == "game_object")
		{
			GameObject* go = WorldManager::Get()->CreateGameObject(
				obj["name"].GetString(),
				WorldManager::Get()->GetTransformProvider().Allocate(),
				WorldManager::Get()->GetTransformProvider()
			);

			rapidjson::Value& transformValue = obj["transform"];
			ParseTransform(go, transformValue);

			for (auto& component : obj["components"].GetArray())
			{
				std::string type = std::string(component["asset_type"].GetString());
				if (type == "renderer")
				{
					bool isStatic = component["static"].GetBool();
					std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>(*go, isStatic);
					mr->SetMesh(component["model"].GetString());
					mr->SetMaterial(component["material"].GetString());
					go->AddComponent(std::move(mr));
				}
				else if (type == "component")
				{
					ASSERT(component.HasMember("component"));
					auto type = component["component"].GetString();
					ASSERT(SerializableClassFactory::GetInstance() != nullptr);
					std::unique_ptr<Serializable> s = SerializableClassFactory::GetInstance()->Deserialize(
						*go, component["fields"], component["component"].GetString());
					auto* c_ptr = JoyCast<Component>(s.release());
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
							0,
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

			parent->AddChild(go);

			ASSERT(obj.HasMember("children"));

			rapidjson::Value& children = obj["children"];
			for (auto& child : children.GetArray())
			{
				ParseGameObjectJson(child, go);
			}
			return go;
		}
		else if (objType == "prefab")
		{
			ASSERT(obj.HasMember("path"));
			rapidjson::Document json = DataManager::Get()->GetSerializedData(obj["path"].GetString(), AssetType::GameObject);
			GameObject* go = ParseGameObjectJson(json, parent);
			rapidjson::Value& transformValue = obj["transform"];
			ParseTransform(go, transformValue);
			return go;
		}
		else
		{
			ASSERT(false);
		}

		return nullptr;
	}

	Scene::Scene(rapidjson::Value& json):
		GameObject(
			json["name"].GetString(),
			WorldManager::Get()->GetTransformProvider().Allocate(),
			WorldManager::Get()->GetTransformProvider())
	{
		rapidjson::Value& val = json["objects"];
		for (auto& obj : val.GetArray())
		{
			ParseGameObjectJson(obj, this);
		}
	}

	void UpdateCycle(GameObject* object)
	{
		if (object == nullptr) return;
		object->Update();
		if (object->GetNextSibling() != nullptr)
		{
			UpdateCycle(object->GetNextSibling());
		}
		if (object->GetFirstChild() != nullptr)
		{
			UpdateCycle(object->GetFirstChild());
		}
	}

	void Scene::Update()
	{
		UpdateCycle(this->m_firstChild);
	}
}
