#include "MtlBinaryParser.h"



#include "Material.h"
#include "DataManager/DataManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "RenderManager/JoyTypes.h"


namespace JoyEngine
{
	MtlBinaryParser::MtlBinaryParser(GUID modelGuid, GUID materialGuid)
	{
		m_modelStream = DataManager::Get()->GetFileStream(modelGuid, false);
		rapidjson::Document json = DataManager::Get()->GetSerializedData(materialGuid, mtl_material);

		rapidjson::Value& val = json["materials"];
		for (auto& mat : val.GetArray())
		{
			std::map<std::string, std::string> bindings{
				{"diffuse", mat["diffuse"].GetString()},
				{"normal", mat["normal"].GetString()}
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


		MeshHeader header = {};
		m_modelStream.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
		m_meshStreamData = {
			static_cast<uint32_t>(header.vertexCount * sizeof(Vertex)),
			static_cast<uint32_t>(header.indexCount * sizeof(uint32_t)),
			static_cast<uint32_t>(m_currentStreamPosition + sizeof(MeshHeader)),
			static_cast<uint32_t>(m_currentStreamPosition + sizeof(MeshHeader) + header.vertexCount * sizeof(Vertex)),
			header.materialIndex
		};

		m_currentStreamPosition +=
			sizeof(MeshHeader) +
			header.indexCount * sizeof(uint32_t) +
			header.vertexCount * sizeof(Vertex);

		return &m_meshStreamData;
	}
}
