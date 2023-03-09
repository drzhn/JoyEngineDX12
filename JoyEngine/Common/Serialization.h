#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <map>
#include <glm/glm.hpp>
#include <rapidjson/document.h>

#include "Utils/Assert.h"
#include "Common/Color.h"
#include "Serializable.h"
#include "HashDefs.h"

//    =============== USAGE: ===================
//    
//		class.h
// 
//    class SomeClass : public Serializable {
//
//        DECLARE_CLASS_NAME(SomeClass)
//
//        REFLECT_FIELD(float, a);
//        REFLECT_FIELD(int, b);
//
//    };
// 
//		class.cpp
// 
//	  DECLARE_CLASS(SomeClass)
// 



#define DECLARE_CLASS(className) \
static SerializedObjectCreator<className> className##_creator = SerializedObjectCreator<className>(#className);

#define DECLARE_CLASS_NAME(T) \
static constexpr const char* className = #T;\
public:\
	T(GameObject& go):Component(go){};\
private:\


// TODO correct check for Serializable class !
#define REFLECT_FIELD(T, v) FieldRegistrator v##_registrator = FieldRegistrator \
(className, #v, FieldInfo(HASH(T) != HASH(Serializable)? HASH(T) : HASH(Serializable), &(v))); T v;

namespace JoyEngine
{
	class GameObject;
	class SerializableClassFactory;

	class SerializedObjectCreatorBase
	{
	public:
		SerializedObjectCreatorBase() = default;

		// TODO pass parameters as template arguments
		// TODO not only Component can be Serializable
		virtual std::unique_ptr<Serializable> Create(GameObject& go) = 0; 
	};

	template <typename Type>
	class SerializedObjectCreator final : public SerializedObjectCreatorBase
	{
	public:
		explicit SerializedObjectCreator(const std::string& className);

		std::unique_ptr<Serializable> Create(GameObject& go) override;
	};

	struct FieldInfo
	{
		FieldInfo() = default;

		FieldInfo(uint32_t typeHash, void* fieldOffset) : typeHash(typeHash), fieldOffset(fieldOffset)
		{
		}

		uint32_t typeHash;
		void* fieldOffset;
	};

	class SerializableClassFactory
	{
	public:
		void RegisterClass(const std::string& className, SerializedObjectCreatorBase* creator);

		void RegisterClassFieldOffset(const std::string& className, const std::string& filedName, FieldInfo fieldInfo);

		std::unique_ptr<Serializable> Deserialize(GameObject& go, rapidjson::Value& fieldsJson, const std::string& className);

		// don't want to make storages static because of exceptions before main()
		static SerializableClassFactory* GetInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new SerializableClassFactory();
			}
			return m_instance;
		}

	private:
		static SerializableClassFactory* m_instance;
		std::map<std::string, std::map<std::string, FieldInfo>> m_fieldOffsetStorage;
		std::map<std::string, SerializedObjectCreatorBase*> m_classCreatorStorage;
	};

	class FieldRegistrator
	{
	public:
		FieldRegistrator(const std::string& className, const std::string& filedName, FieldInfo fieldInfo)
		{
			SerializableClassFactory::GetInstance()->RegisterClassFieldOffset(className, filedName, fieldInfo);
		}
	};

	template <typename Type>
	SerializedObjectCreator<Type>::SerializedObjectCreator(const std::string& className)
	{
		SerializableClassFactory::GetInstance()->RegisterClass(className, this);
	}

	template <typename Type>
	std::unique_ptr<Serializable> SerializedObjectCreator<Type>::Create(GameObject& go)
	{
		std::unique_ptr<Type> asset = std::make_unique<Type>(go);
		ASSERT(dynamic_cast<Serializable*>(asset.get()) != nullptr);
		return std::unique_ptr<Type>(std::move(asset));
	}
}

#endif //SERIALIZATION_H
