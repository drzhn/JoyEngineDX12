#pragma once

#include "Common/HashDefs.h"

struct TypeInfo
{
	const TypeInfo* parentInfo;

	const char* typeName;
	const uint64_t typeId;
};

#define DECLARE_JOY_OBJECT(T, BASE) \
protected:																			   \
static constexpr TypeInfo typeInfo{		   											   \
	.parentInfo = &(BASE##::typeInfo),				   								   \
	.typeName = #T,						   											   \
	.typeId = StrHash64(#T)				   											   \
};										   											   \
public:																				   \
constexpr uint64_t GetClassId() override { return typeInfo.typeId; } 				   \
constexpr const char* GetClassname() override { return typeInfo.typeName; } 		   \
constexpr const TypeInfo* GetTypeInfo() override { return &typeInfo; }                 \
private:

namespace JoyEngine
{
	class JoyObject
	{
	protected:
		static constexpr TypeInfo typeInfo{
			.parentInfo = nullptr,
			.typeName = "JoyObject",
			.typeId = StrHash64("JoyObject")
		};

	public:
		constexpr virtual const char* GetClassname() { return typeInfo.typeName; }
		constexpr virtual uint64_t GetClassId() = 0;
		constexpr virtual const TypeInfo* GetTypeInfo() { return &typeInfo; }

	protected:
		virtual ~JoyObject() = default;
	};

	template <typename T>
	constexpr std::string_view GetTypeName()
	{
#ifdef __clang__
		std::string_view sv = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
		std::string_view sv = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
		std::string_view sv = __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		sv = sv.substr(sv.find("class JoyEngine::") + 17);
		sv = sv.substr(0, sv.find('>'));
		return sv;
	}

	template <typename T> requires std::is_base_of_v<JoyObject, T>
	constexpr T* JoyCast(JoyObject* obj)
	{
		constexpr std::string_view sv = GetTypeName<T>();

		const TypeInfo* typeInfo = obj->GetTypeInfo();
		while (typeInfo != nullptr)
		{
			if (typeInfo->typeId == StrnHash64(sv.data(), sv.length()))
			{
				return static_cast<T*>(obj);
			}
			typeInfo = typeInfo->parentInfo;
		}
		return nullptr;
	}
}
