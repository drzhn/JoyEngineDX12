#include "MtlBinaryParser.h"


#include "JoyContext.h"
#include "Material.h"
#include "DataManager/DataManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "RenderManager/JoyTypes.h"


namespace JoyEngine
{
	MtlBinaryParser::MtlBinaryParser(GUID modelGuid, GUID materialGuid)
	{
		m_modelStream = JoyContext::Data->GetFileStream(modelGuid, false);
		rapidjson::Document json = JoyContext::Data->GetSerializedData(materialGuid, mtl_material);

		rapidjson::Value& val = json["materials"];
		for (auto& mat : val.GetArray())
		{
			std::map<std::string, std::string> bindings{
				{"diffuse", mat["diffuse"].GetString()},
				{"normal", mat["normal"].GetString()}
			}; // TODO values is paths, not GUIDS!
			Material* material = JoyContext::Resource->LoadResource<Material>(
				GUID::Random(),  
				JoyContext::EngineMaterials->GetStandardSharedMaterial(), 
				bindings
			);
		}

		//m_modelStream.seekg(0);
		//uint32_t pos = 0;
		//int i = 0;
		//while (m_modelStream.peek() != EOF)
		//{
		//	m_modelStream.seekg(pos);
		//	MeshHeader header={};
		//	m_modelStream.read(reinterpret_cast<char*>(&header), sizeof(MeshHeader));
		//	pos += sizeof(MeshHeader) + header.indexCount * sizeof(uint32_t) + header.vertexCount * sizeof(Vertex);
		//	i++;
		//}
	}
}
