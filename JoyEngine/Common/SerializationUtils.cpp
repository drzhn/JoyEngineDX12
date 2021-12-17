#include "SerializationUtils.h"

#include <glm/glm.hpp>
#include "Color.h"

#include "HashDefs.h"

namespace JoyEngine
{
	const std::map<uint32_t, size_t> SerializationUtils::m_typeSizes = {
		{strHash("int"), sizeof(int32_t)},
		{strHash("uint"), sizeof(uint32_t)},
		{strHash("float"), sizeof(float)},
		{strHash("vec2"), sizeof(glm::vec2)},
		{strHash("vec3"), sizeof(glm::vec3)},
		{strHash("vec4"), sizeof(glm::vec4)},
		{strHash("mat3"), sizeof(glm::mat3)},
		{strHash("mat4"), sizeof(glm::mat4)},
		{strHash("color"), sizeof(Color)},

	};

	const std::map<uint32_t, uint32_t> SerializationUtils::m_typeMapping = {
		{HASH(int32_t), strHash("int")},
		{HASH(uint32_t), strHash("uint")},
		{HASH(float), strHash("float")},
		{HASH(glm::vec2), strHash("vec2")},
		{HASH(glm::vec3), strHash("vec3")},
		{HASH(glm::vec4), strHash("vec4")},
		{HASH(glm::mat3), strHash("mat3")},
		{HASH(glm::mat4), strHash("mat4")},
		{HASH(Color), strHash("color")},
	};

	size_t SerializationUtils::GetTypeSize(const std::string& type)
	{
		return GetTypeSize(strHash(type.c_str()));
	}

	size_t SerializationUtils::GetTypeSize(const char* type)
	{
		return GetTypeSize(strHash(type));
	}

	size_t SerializationUtils::GetTypeSize(uint32_t typeHash)
	{
		ASSERT(m_typeSizes.find(typeHash) != m_typeSizes.end());
		return m_typeSizes.find(typeHash)->second;
	}

	size_t SerializationUtils::GetType(uint32_t cppTypeHash)
	{
		ASSERT(m_typeMapping.find(cppTypeHash) != m_typeMapping.end());
		return m_typeMapping.find(cppTypeHash)->second;
	}

	template <typename T>
	inline void Read(const rapidjson::Value& val, void* ptr)
	{
		ASSERT_DESC(false, "Unsupported type");
	}

	template <>
	inline void Read<int32_t>(const rapidjson::Value& val, void* ptr)
	{
		ASSERT(val.IsInt());
		int32_t f = val.GetInt();
		memcpy(ptr, &f, sizeof(int32_t));
	}

	template <>
	inline void Read<uint32_t>(const rapidjson::Value& val, void* ptr)
	{
		ASSERT(val.IsUint());
		uint32_t f = val.GetUint();
		memcpy(ptr, &f, sizeof(uint32_t));
	}

	template <>
	inline void Read<float>(const rapidjson::Value& val, void* ptr)
	{
		float f = 0;
		if (val.IsFloat())
			f = val.GetFloat();
		else if (val.IsInt())
			f = static_cast<float>(val.GetInt());
		else if (val.IsUint())
			f = static_cast<float>(val.GetUint());
		else
			ASSERT(false);
		memcpy(ptr, &f, sizeof(float));
	}

	template <typename T>
	void ReadArray(const rapidjson::Value& val, void* ptr, uint32_t count)
	{
		ASSERT(val.IsArray());
		ASSERT(val.GetArray().Size() == count);
		ptrdiff_t offset = 0;
		for (const auto& v : val.GetArray())
		{
			Read<T>(v, static_cast<char*>(ptr) + offset);
			offset += sizeof(T);
		}
	}

	// for example, if value is single vector, we store it it 1-dim array: "position": [0.2, 10, 0.56]
	// but if value is array of vectors, we store it in 2-dim array: "positions": [[0.2, 10, 0.56],[3, 16, 23]]
	template <typename T>
	void ReadStruct(const rapidjson::Value& val, void* ptr, uint32_t rowsCount, uint32_t columnsCount)
	{
		if (rowsCount == 1)
		{
			ReadArray<T>(val, ptr, columnsCount);
		}
		else
		{
			ASSERT(val.IsArray());
			ASSERT(val.GetArray().Size() == rowsCount);
			size_t offset = 0;
			for (const auto& v : val.GetArray())
			{
				ReadArray<T>(v, static_cast<char*>(ptr) + offset, columnsCount);
				offset += sizeof(T) * columnsCount;
			}
		}
	}

	void SerializationUtils::DeserializeAndWriteCppToPtr(uint32_t typeHash, const rapidjson::Value& val,
	                                                     void* ptr)
	{
		DeserializeToPtr(GetType(typeHash), val, ptr);
	}

	void SerializationUtils::DeserializeToPtr(uint32_t typeHash,
	                                          const rapidjson::Value& val, void* ptr,
	                                          uint32_t count)
	{
		ASSERT(count > 0);

		switch (typeHash)
		{
		case strHash("int"):
			{
				if (count == 1)
				{
					Read<int32_t>(val, ptr);
				}
				else
				{
					ReadArray<int32_t>(val, ptr, count);
				}
				break;
			}
		case strHash("uint"):
			{
				if (count == 1)
				{
					Read<uint32_t>(val, ptr);
				}
				else
				{
					ReadArray<uint32_t>(val, ptr, count);
				}
				break;
			}
		case strHash("float"):
			{
				if (count == 1)
				{
					Read<float>(val, ptr);
				}
				else
				{
					ReadArray<float>(val, ptr, count);
				}
				break;
			}
		case strHash("vec2"):
			{
				ReadStruct<float>(val, ptr, count, 2);
				break;
			}
		case strHash("vec3"):
			{
				ReadStruct<float>(val, ptr, count, 3);
				break;
			}
		case strHash("vec4"):
		case strHash("color"):
			{
				ReadStruct<float>(val, ptr, count, 4);
				break;
			}
		case strHash("mat3"):
			{
				ReadStruct<float>(val, ptr, count, 9);
				break;
			}
		case strHash("mat4"):
			{
				ReadStruct<float>(val, ptr, count, 16);
				break;
			}
		default:
			ASSERT(false);
		}
	}
}
