#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <vector>

#include "JoyAssetHeaders.h"

class TextureLoader
{
public:
	[[nodiscard]] bool LoadTexture(const std::string& filename, std::string& errorMessage);
	[[nodiscard]] bool WriteData(const std::string& dataFilename, std::string& errorMessage) const;
private:
	struct TextureData
	{
		TextureAssetHeader m_header;
		std::vector<char> m_data;
	};

	TextureData m_currentTextureData = {};
};

#endif //TEXTURE_LOADER_H
