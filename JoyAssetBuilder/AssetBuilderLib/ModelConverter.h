#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <fstream>
#include <vector>

#include "fbxsdk.h"

#include "CommonEngineStructs.h"
#include "JoyAssetHeaders.h"

namespace JoyEngine
{
	struct ShapeData
	{
		MeshAssetHeader m_header;
		std::vector<Vertex> m_vertices;
		std::vector<Index> m_indices;
		size_t GetSizeInBytes() const
		{
			if (m_header.vertexDataSize == 0 || m_header.indexDataSize == 0)
			{
				// we will not write this data to the output file;
				return 0;
			}
			return sizeof(MeshAssetHeader) + m_header.vertexDataSize + m_header.indexDataSize;
		}
	};

	class ModelConverter
	{
	public:
		ModelConverter();
		~ModelConverter();
		[[nodiscard]] bool ConvertModel(const std::string& modelPath, const std::string& dataDir, std::string& errorMessage);

	private:

		FbxManager* m_lSdkManager;
		FbxImporter* m_lImporter;
		std::unique_ptr<FbxGeometryConverter> m_lGeometryConverter;
	};
}


#endif //MODEL_LOADER_H
