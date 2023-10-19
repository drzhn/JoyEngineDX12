#include "Serialization.h"

#include "SerializationUtils.h"

namespace JoyEngine
{
	SerializableClassFactory* SerializableClassFactory::m_instance = nullptr;

	void SerializableClassFactory::RegisterClass(const std::string& className, SerializedObjectCreatorBase* creator)
	{
		GetInstance()->m_classCreatorStorage.insert(std::make_pair(className, creator));
	}

	void SerializableClassFactory::RegisterClassFieldOffset(const std::string& className, const std::string& filedName,
	                                                        FieldInfo fieldInfo)
	{
		if (GetInstance()->m_fieldOffsetStorage.find(className) == GetInstance()->m_fieldOffsetStorage.end())
		{
			GetInstance()->m_fieldOffsetStorage.insert({className, std::map<std::string, FieldInfo>()});
		}
		if (GetInstance()->m_fieldOffsetStorage[className].find(filedName) == GetInstance()->m_fieldOffsetStorage[
			className].end())
		{
			GetInstance()->m_fieldOffsetStorage[className].insert({filedName, fieldInfo});
		}
		else
		{
			GetInstance()->m_fieldOffsetStorage[className][filedName].fieldOffset = fieldInfo.fieldOffset;
		}
	}

	std::unique_ptr<Serializable> SerializableClassFactory::Deserialize(
		GameObject& go,
		rapidjson::Value& fieldsJson,
		const std::string& className)
	{
		ASSERT(GetInstance()->m_classCreatorStorage.contains(className));

		SerializedObjectCreatorBase* creator = GetInstance()->m_classCreatorStorage.find(className)->second;
		std::unique_ptr<Serializable> object = creator->Create(go);
		ASSERT(GetInstance()->m_fieldOffsetStorage.contains(className));

		auto fieldOffsetStorage = GetInstance()->m_fieldOffsetStorage[className];
		for (auto member = fieldsJson.MemberBegin(); member != fieldsJson.MemberEnd(); ++member)
		{
			const std::string fieldName = member->name.GetString();
			uint32_t typeHash = fieldOffsetStorage[fieldName].typeHash;
			void* fieldOffset = fieldOffsetStorage[fieldName].fieldOffset;
			rapidjson::Value& val = member->value;
			SerializationUtils::DeserializeAndWriteCppToPtr(typeHash, val, fieldOffset);
		}
		return std::move(object);
	}
}
