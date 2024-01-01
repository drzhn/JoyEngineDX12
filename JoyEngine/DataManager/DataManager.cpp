#include "DataManager.h"

#include <fstream>
#include <vector>

#include <rapidjson/document.h>

#include "JoyAssetHeaders.h"
#include "Common/HashDefs.h"
#include "Utils/FileUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	DataManager::DataManager() : m_dataPath(std::filesystem::absolute(R"(JoyData/)"))
	{
		TIME_PERF("DataManager ctor")
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

	void DataManager::GetWFilename(const std::string& path, std::wstring& filename)
	{
		filename = std::filesystem::path(path).stem().generic_wstring();
	}

	std::ifstream DataManager::GetFileStream(const std::string& path, bool shouldReadRawData) const
	{
		const auto delimiterPos = path.find_first_of(':');
		std::string dataPath = path;
		if (delimiterPos != std::string::npos)
		{
			dataPath = dataPath.substr(0, delimiterPos);
		}
		dataPath += shouldReadRawData ? ".data" : "";
		std::ifstream stream = GetStream((m_dataPath / dataPath).generic_string());

		if (delimiterPos != std::string::npos)
		{
			std::stringstream treePath = std::stringstream(path.substr(delimiterPos + 1));

			std::string nodeName;
			uint32_t nodeIndex = 0;
			uint32_t childCount = 1;
			TreeEntry entry = {};
			while (std::getline(treePath, nodeName, '/'))
			{
				stream.seekg(sizeof(TreeEntry) * nodeIndex);
				bool found = false;
				for (uint32_t i = 0; i < childCount; i++)
				{
					stream.read(reinterpret_cast<char*>(&entry), sizeof(TreeEntry));
					if (entry.nameHash == StrHash64(nodeName.c_str()))
					{
						nodeIndex = entry.childStartIndex;
						childCount = entry.childCount;
						found = true;
						break;
					}
				}
				ASSERT(found);
			}

			stream.seekg(entry.dataFileOffset);
		}

		return stream;
	}

	// TODO rewrite using template<ResourceT>
	rapidjson::Document DataManager::GetSerializedData(const std::string& path, AssetType type) const
	{
		const std::vector<char> data = GetData(path);
		rapidjson::Document json;
		json.Parse<rapidjson::kParseStopWhenDoneFlag>(data.data());

#ifdef _DEBUG
		std::string s;
		switch (type)
		{
		case AssetType::Mesh:
			s = "mesh";
			break;
		case AssetType::Texture:
			s = "texture";
			break;
		case AssetType::Shader:
			s = "shader";
			break;
		case AssetType::Material:
			s = "material";
			break;
		case AssetType::SharedMaterial:
			s = "sharedMaterial";
			break;
		case AssetType::Scene:
			s = "scene";
			break;
		case AssetType::GameObject:
			s = "game_object";
			break;
		case AssetType::Prefab:
			s = "prefab";
			break;
		case AssetType::Skybox:
			s = "skybox";
			break;
		case AssetType::World:
			s = "world";
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
