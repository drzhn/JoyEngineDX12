#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <fstream>
#include <vector>

#include "fbxsdk.h"

#include "CommonEngineStructs.h"
#include "JoyAssetHeaders.h"

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();
	[[nodiscard]] bool LoadModel(const std::string& modelFilename, const std::string& materialsDir, std::string& errorMessage);
	[[nodiscard]] bool WriteData(const std::string& dataFilename, std::string& errorMessage) const;

private:
	struct ShapeData
	{
		MeshAssetHeader m_header;
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
	};

	std::vector<ShapeData> m_shapes;

	FbxManager* m_lSdkManager;
	FbxImporter* m_lImporter;
};


#endif //MODEL_LOADER_H
