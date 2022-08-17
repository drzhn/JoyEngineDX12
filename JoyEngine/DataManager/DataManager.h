#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <map>
#include <filesystem>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"
#include "Utils/GUID.h"
#include "Common/Singleton.h"


namespace JoyEngine
{
	enum DataType
	{
		mesh,
		texture,
		shader,
		material,
		mtl_material,
		sharedMaterial,
		scene,
	};

	class DataManager : public Singleton<DataManager>
	{
	public:
		DataManager();
		~DataManager() = default;

		[[nodiscard]] std::vector<char> GetData(GUID guid, bool shouldReadRawData = false, uint32_t offset = 0) const;
		[[nodiscard]] bool HasRawData(GUID guid) const;
		[[nodiscard]] bool HasRawData(const std::string& path) const;
		[[nodiscard]] std::ifstream GetFileStream(GUID guid, bool shouldReadRawData = false) const;
		[[nodiscard]] std::ifstream GetFileStream(const std::string& path, bool shouldReadRawData = false) const;
		[[nodiscard]] rapidjson::Document GetSerializedData(const GUID&, DataType) const;
		[[nodiscard]] std::filesystem::path GetAbsolutePath(GUID) const;

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
