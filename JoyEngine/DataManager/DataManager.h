#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <filesystem>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"
#include "Utils/GUID.h"


namespace JoyEngine
{
	enum DataType
	{
		mesh,
		texture,
		shader,
		material,
		sharedMaterial,
		scene
	};

	class DataManager
	{
	public:
		DataManager();

		~DataManager();

		std::vector<char> GetData(GUID guid, bool shouldReadRawData = false)
		{
			ASSERT(m_pathDatabase.find(guid) != m_pathDatabase.end());
			std::string filename = m_dataPath + m_pathDatabase[guid].string();
			if (shouldReadRawData)
			{
				filename += ".data";
			}
			return ReadFile(filename);
		}

		std::ifstream GetFileStream(GUID guid, bool shouldReadRawData = false)
		{
			ASSERT(m_pathDatabase.find(guid) != m_pathDatabase.end());
			std::string filename = m_dataPath + m_pathDatabase[guid].string();
			if (shouldReadRawData)
			{
				filename += ".data";
			}
			return GetStream(filename);
		}

		rapidjson::Document GetSerializedData(const GUID&, DataType);

		std::filesystem::path GetAbsolutePath(GUID);

	private:
		const std::string m_dataPath = R"(D:\CppProjects\JoyEngineDX\JoyData\)";
		const std::string m_databaseFilename;
		std::map<GUID, std::filesystem::path> m_pathDatabase;

	private:
		const std::filesystem::path& GetPath(GUID);

		void ParseDatabase(std::map<GUID, std::filesystem::path>& pathDatabase, const char* data);
	};
}

#endif //DATA_MANAGER_H
