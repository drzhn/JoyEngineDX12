#ifndef MTL_BINARY_PARSER_H
#define MTL_BINARY_PARSER_H

#include <fstream>
#include <vector>

#include "ResourceManager/ResourceManager.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Material;

	struct MtlMeshStreamData
	{
		uint32_t vertexDataSize;
		uint32_t indexDataSize;
		uint32_t vertexStreamOffset;
		uint32_t indexStreamOffset;
		uint32_t materialIndex;
	};

	class MtlBinaryParser
	{
	public:
		MtlBinaryParser(GUID modelGuid, GUID materialGuid);
		std::ifstream& GetModelStream();
		ResourceHandle<Material> GetMaterialByIndex(uint32_t index);
		MtlMeshStreamData* Next();
	private:
		std::ifstream m_modelStream;
		std::vector<ResourceHandle<Material>> m_materials;
		bool m_reachedEnd = false;
		uint32_t m_currentStreamPosition = 0;
		MtlMeshStreamData m_meshStreamData;
	};
}

#endif // MTL_BINARY_PARSER_H
