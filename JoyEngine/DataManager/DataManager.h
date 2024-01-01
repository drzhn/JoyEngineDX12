#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <filesystem>

#include <rapidjson/document.h>

#include "Utils/FileUtils.h"
#include "Common/Singleton.h"


namespace JoyEngine
{
	enum class AssetType
	{
		Mesh,
		Texture,
		Shader,
		Material,
		SharedMaterial,
		Scene,
		GameObject,
		Prefab,
		Skybox,
		World,
	};

	class DataManager : public Singleton<DataManager>
	{
	public:
		DataManager();
		~DataManager() = default;

		[[nodiscard]] std::vector<char> GetData(const std::string& path, bool shouldReadRawData = false, uint32_t offset = 0) const;
		bool HasRawData(const std::string& path) const;
		void GetWFilename(const std::string& path, std::wstring& filename);
		[[nodiscard]] std::ifstream GetFileStream(const std::string& path, bool shouldReadRawData = false) const;
		[[nodiscard]] rapidjson::Document GetSerializedData(const std::string& path, AssetType) const;
	private:
		const std::filesystem::path m_dataPath;
	};
}

#endif //DATA_MANAGER_H
