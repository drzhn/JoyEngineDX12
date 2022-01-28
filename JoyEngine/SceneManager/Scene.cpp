#include "Scene.h"

#include "rapidjson/document.h"

#include "JoyContext.h"
#include "Common/Serialization.h"
#include "Common/SerializationUtils.h"
#include "Components/Component.h"
#include "Components/MeshRenderer.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "DataManager/DataManager.h"

namespace JoyEngine
{
	Scene::Scene(const GUID& guid)
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(guid, scene);
		m_name = json["name"].GetString();

		rapidjson::Value& val = json["objects"];
		for (auto& obj : val.GetArray())
		{
			std::unique_ptr<GameObject> go = std::make_unique<GameObject>(obj["name"].GetString());

			rapidjson::Value& transformValue = obj["transform"];
			glm::vec3 vec;
			SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localPosition"], &vec, 1);
			go->GetTransform()->SetPosition(vec);
			SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localRotation"], &vec, 1);
			go->GetTransform()->SetRotation(vec);
			SerializationUtils::DeserializeToPtr(strHash("vec3"), transformValue["localScale"], &vec, 1);
			go->GetTransform()->SetScale(vec);

			for (auto& component : obj["components"].GetArray())
			{
				std::string type = std::string(component["type"].GetString());
				if (type == "renderer")
				{
					std::unique_ptr<MeshRenderer> mr = std::make_unique<MeshRenderer>();
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
						component["fields"], component["component"].GetString());
					auto* c_ptr = dynamic_cast<Component*>(s.release());
					ASSERT(c_ptr != nullptr);
					std::unique_ptr<Component> c(c_ptr);
					go->AddComponent(std::move(c));
				}
				else if (type == "camera")
				{
					go->AddComponent(std::make_unique<Camera>());
				}
				else if (type == "light")
				{
					std::unique_ptr<Light> light;
					std::string lightTypeStr = std::string(component["lightType"].GetString());
					if (lightTypeStr == "point")
					{
						float intensity = component["intensity"].GetFloat();
						float radius = component["radius"].GetFloat();

						light = std::make_unique<Light>(LightType::Point, intensity, radius, 0, 0);
					}
					else
					{
						ASSERT(false);
					}
					go->AddComponent(std::move(light));
				}
			}
			m_objects.push_back(std::move(go));
		}
	}

	void Scene::Update()
	{
		for (const auto& o : m_objects)
		{
			o->Update();
		}
	}
}
