#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <Blob.h>
#include <string>
#include <vector>

#include "JoyAssetHeaders.h"

class TextureLoader
{
public:
	TextureLoader();
	[[nodiscard]] bool LoadTexture(const std::string& filePath, std::string& errorMessage);
	[[nodiscard]] bool WriteData(const std::string& dataFilename, std::string& errorMessage) const;
private:
	struct TextureData
	{
		TextureAssetHeader m_header;
		Blob blob;
	};

	TextureData m_currentTextureData = {};
};

#endif //TEXTURE_LOADER_H
