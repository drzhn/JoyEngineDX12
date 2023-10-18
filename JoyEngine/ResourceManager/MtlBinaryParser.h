#ifndef MTL_BINARY_PARSER_H
#define MTL_BINARY_PARSER_H

#include <fstream>
#include <vector>

#include "Material.h"
#include "ResourceManager/ResourceManager.h"


namespace JoyEngine
{
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
		MtlBinaryParser(const std::string& modelGuid, const std::string& materialGuid);
		std::ifstream& GetModelStream();
		ResourceHandle<Material> GetMaterialByIndex(uint32_t index);
		MtlMeshStreamData* Next();
	private:
		std::ifstream m_modelStream;
		std::vector<ResourceHandle<Material>> m_materials;
		bool m_reachedEnd = false;
		uint32_t m_currentStreamPosition = 0;
		MtlMeshStreamData m_meshStreamData = {};
	};
}

#endif // MTL_BINARY_PARSER_H
