#include "MtlBinaryParser.h"

#include <JoyAssetHeaders.h>


#include "Material.h"
#include "DataManager/DataManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "CommonEngineStructs.h"


namespace JoyEngine
{
	MtlBinaryParser::MtlBinaryParser(GUID modelGuid, GUID materialGuid)
	{
		m_modelStream = DataManager::Get()->GetFileStream(modelGuid, false);
		rapidjson::Document json = DataManager::Get()->GetSerializedData(materialGuid, standard_material_list);

		rapidjson::Value& val = json["materials"];
		for (auto& mat : val.GetArray())
		{
			std::map<std::string, std::string> bindings{
				{"diffuse", mat["diffuse"].GetString()},
				//{"normal", mat["normal"].GetString()}
			};
			m_materials.emplace_back(ResourceManager::Get()->LoadResource<Material>(
				GUID::Random(),
				EngineMaterialProvider::Get()->GetStandardSharedMaterial(),
				bindings,
				true
			));
		}

		//m_modelStream.seekg(0);
		//uint32_t pos = 0;
		//int i = 0;
		//while (m_modelStream.peek() != EOF)
		//{
		//	m_modelStream.clear();
		//	m_modelStream.seekg(pos);
		//	MeshHeader header={};
		//	m_modelStream.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
		//	pos += sizeof(MeshHeader) + header.indexCount * sizeof(uint32_t) + header.vertexCount * sizeof(Vertex);
		//	i++;
		//}
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
