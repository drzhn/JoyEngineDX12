#ifndef SERIALIZATION_UTILS_H
#define SERIALIZATION_UTILS_H

#include <map>
#include <xstring>
#include <rapidjson/document.h>

namespace JoyEngine
{
	class SerializationUtils
	{
	public:
		static size_t GetTypeSize(const char* type);
		static size_t GetTypeSize(uint32_t typeHash);
		static size_t GetTypeSize(const std::string& type);

		static size_t GetType(uint32_t cppTypeHash);
		static void DeserializeAndWriteCppToPtr(uint32_t typeHash, const rapidjson::Value& val, void* ptr);
		static void DeserializeToPtr(uint32_t typeHash, const rapidjson::Value&, void* ptr, uint32_t count=1);
	private:
		// size of standard serializable type
		static const std::map<uint32_t, size_t> m_typeSizes;
		// map from real c++ type to serializable type e.g glm::vec4 -> color
		static const std::map<uint32_t, uint32_t> m_typeMapping;
	};
}
#endif // SERIALIZATION_UTILS_H
