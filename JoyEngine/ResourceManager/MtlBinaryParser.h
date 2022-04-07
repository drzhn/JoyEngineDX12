#ifndef MTL_BINARY_PARSER_H
#define MTL_BINARY_PARSER_H

#include <fstream>
#include <vector>

#include "ResourceHandle.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Material;

	class MtlBinaryParser
	{
	public:
		MtlBinaryParser(GUID modelGuid, GUID materialGuid);
	private:
		std::ifstream m_modelStream;
		std::vector<ResourceHandle<Material>> m_materials;
	};
}

#endif // MTL_BINARY_PARSER_H
