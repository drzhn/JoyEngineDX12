#include "MtlBinaryParser.h"

#include <JoyAssetHeaders.h>


#include "Material.h"
#include "DataManager/DataManager.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "CommonEngineStructs.h"


namespace JoyEngine
{
	MtlBinaryParser::MtlBinaryParser(const std::string& modelGuid, const std::string& materialGuid)
	{
		m_modelStream = DataManager::Get()->GetFileStream(modelGuid, true);
		rapidjson::Document json = DataManager::Get()->GetSerializedData(materialGuid, standard_material_list);

		rapidjson::Value& val = json["materials"];
		for (auto& mat : val.GetArray())
		{
			m_materials.emplace_back(
				ResourceManager::Get()->RegisterResource<Material>(
					new Material(
						RandomHash64(), // todo temprorary solution. we will not have multiple textures in future
						mat
					)
				)
			);
		}
	}

	std::ifstream& MtlBinaryParser::GetModelStream()
	{
		return m_modelStream;
	}

	ResourceHandle<Material> MtlBinaryParser::GetMaterialByIndex(uint32_t index)
	{
		return m_materials.at(index);
	}

	MtlMeshStreamData* MtlBinaryParser::Next()
	{
		bool a = m_modelStream.is_open();
		m_modelStream.clear();
		m_modelStream.seekg(m_currentStreamPosition);
		if (m_modelStream.peek() == EOF)
		{
			m_reachedEnd = true;
			return nullptr;
		}


		MeshAssetHeader header = {};
		m_modelStream.read(reinterpret_cast<char*>(&header), sizeof(MeshAssetHeader));
		m_meshStreamData = {
			header.vertexDataSize,
			header.indexDataSize,
			static_cast<uint32_t>(m_currentStreamPosition + sizeof(MeshAssetHeader)),
			static_cast<uint32_t>(m_currentStreamPosition + sizeof(MeshAssetHeader) + header.vertexDataSize),
			header.materialIndex
		};

		m_currentStreamPosition +=
			sizeof(MeshAssetHeader) +
			header.vertexDataSize +
			header.indexDataSize;

		return &m_meshStreamData;
	}
}
