#include "DataManager.h"
#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"

namespace JoyEngine
{
	DataManager::DataManager() :
		//m_databaseFilename(R"(data_old.db)")
		m_databaseFilename(R"(data.db)")
	{
		ParseDatabase(m_pathDatabase, ReadFile(m_dataPath + m_databaseFilename).data());
	}

	DataManager::~DataManager()
	{
	}

	const std::filesystem::path& DataManager::GetPath(GUID guid)
	{
		if (m_pathDatabase.find(guid) == m_pathDatabase.end())
		{
			ASSERT(false);
		}
		return m_pathDatabase[guid];
	}

	std::filesystem::path DataManager::GetAbsolutePath(GUID guid)
	{
		if (m_pathDatabase.find(guid) == m_pathDatabase.end())
		{
			ASSERT(false);
		}
		std::filesystem::path root = m_dataPath;
		root += m_pathDatabase[guid];
		return root;
	}

	void DataManager::ParseDatabase(std::map<GUID, std::filesystem::path>& pathDatabase, const char* data)
	{
		rapidjson::Document json;
		json.Parse<rapidjson::kParseStopWhenDoneFlag>(data);
		ASSERT(json["type"].GetString() == std::string("database"));
		rapidjson::Value& val = json["database"];
		for (auto& v : val.GetArray())
		{
			pathDatabase.insert({
				GUID::StringToGuid(v["guid"].GetString()),
				v["path"].GetString()
			});
		}
	}

	rapidjson::Document DataManager::GetSerializedData(const GUID& sharedMaterialGuid, DataType type)
	{
		std::vector<char> data = GetData(sharedMaterialGuid);
		rapidjson::Document json;
		json.Parse<rapidjson::kParseStopWhenDoneFlag>(data.data());

#ifdef _DEBUG
		std::string s;
		switch (type)
		{
		case mesh:
			s = "mesh";
			break;
		case texture:
			s = "texture";
			break;
		case shader:
			s = "shader";
			break;
		case material:
			s = "material";
			break;
		case sharedMaterial:
			s = "sharedMaterial";
			break;
		case scene:
			s = "scene";
			break;
		case mtl_material:
			s = "mtl_material";
			break;
		default:
			ASSERT(false);
		}

		ASSERT(json.HasMember("type"));
		ASSERT(json["type"].GetString() == std::string(s));
#endif

		return json;
	}
}
