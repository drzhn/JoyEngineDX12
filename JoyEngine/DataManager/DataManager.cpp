#include "DataManager.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <dxcapi.h>

#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>
#include <wrl/client.h>

#include "Utils/FileUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	IMPLEMENT_SINGLETON(DataManager)

	DataManager::DataManager() :
		m_dataPath(std::filesystem::absolute(R"(JoyData/)").generic_string()),
		m_databaseFilename(R"(data.db)")
	{
		TIME_PERF("DataManager init")

		ParseDatabase(m_pathDatabase, ReadFile(m_dataPath + m_databaseFilename).data());
	}

	std::vector<char> DataManager::GetData(GUID guid, bool shouldReadRawData, uint32_t offset) const
	{
		ASSERT(m_pathDatabase.find(guid) != m_pathDatabase.end());
		std::string filename = m_dataPath + m_pathDatabase.find(guid)->second.string();
		if (shouldReadRawData)
		{
			filename += ".data";
		}
		return ReadFile(filename, offset);
	}

	bool DataManager::HasRawData(GUID guid) const
	{
		ASSERT(m_pathDatabase.find(guid) != m_pathDatabase.end());
		std::string filename = m_dataPath + m_pathDatabase.find(guid)->second.string();
		filename += ".data";
		return std::filesystem::exists(filename);
	}

	bool DataManager::HasRawData(const std::string& path) const
	{
		const std::string filename = m_dataPath + path + ".data";
		return std::filesystem::exists(filename);
	}

	std::ifstream DataManager::GetFileStream(GUID guid, bool shouldReadRawData) const
	{
		ASSERT(m_pathDatabase.find(guid) != m_pathDatabase.end());
		std::string filename = m_dataPath + m_pathDatabase.find(guid)->second.string();
		if (shouldReadRawData)
		{
			filename += ".data";
		}
		return GetStream(filename);
	}

	std::ifstream DataManager::GetFileStream(const std::string& path, bool shouldReadRawData) const
	{
		std::string filename = m_dataPath + path;
		if (shouldReadRawData)
		{
			filename += ".data";
		}
		return GetStream(filename);
	}

	const std::filesystem::path& DataManager::GetPath(GUID guid)
	{
		if (m_pathDatabase.find(guid) == m_pathDatabase.end())
		{
			ASSERT(false);
		}
		return m_pathDatabase[guid];
	}

	std::filesystem::path DataManager::GetAbsolutePath(GUID guid) const
	{
		if (m_pathDatabase.find(guid) == m_pathDatabase.end())
		{
			ASSERT(false);
		}
		std::filesystem::path root = m_dataPath;
		root += m_pathDatabase.find(guid)->second;
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

	rapidjson::Document DataManager::GetSerializedData(const GUID& sharedMaterialGuid, DataType type) const
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
