#include "DataManager.h"

#include <fstream>
#include <vector>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	DataManager::DataManager() : m_dataPath(std::filesystem::absolute(R"(JoyData/)"))
	{
		TIME_PERF("DataManager init")
	}

	std::vector<char> DataManager::GetData(const std::string& path, bool shouldReadRawData, uint32_t offset) const
	{
		if (shouldReadRawData)
		{
			return ReadFile((m_dataPath / (path + ".data")).generic_string(), offset);
		}
		else
		{
			return ReadFile((m_dataPath / path).generic_string(), offset);
		}
	}

	bool DataManager::HasRawData(const std::string& path) const
	{
		return std::filesystem::exists(m_dataPath / (path + ".data"));
	}

	std::ifstream DataManager::GetFileStream(const std::string& path, bool shouldReadRawData) const
	{
		if (shouldReadRawData)
		{
			return GetStream((m_dataPath / (path + ".data")).generic_string());
		}
		else
		{
			return GetStream((m_dataPath / path).generic_string());
		}
	}

	// TODO rewrite using template<ResourceT>
	rapidjson::Document DataManager::GetSerializedData(const std::string& path, DataType type) const
	{
		const std::vector<char> data = GetData(path);
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
		case standard_material_list:
			s = "standard_material_list";
			break;
		default:
			ASSERT(false);
		}

		ASSERT(json.HasMember("asset_type"));
		ASSERT(json["asset_type"].GetString() == std::string(s));
#endif

		return json;
	}
}
