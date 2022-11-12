#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <fstream>

bool TextureLoader::LoadTexture(const std::string& filename, std::string& errorMessage)
{
	int texChannels;
	bool isHdr = stbi_is_hdr(filename.c_str());

	size_t dataSize = 0;
	void* dataPtr = nullptr;
	if (isHdr)
	{
		stbi_set_flip_vertically_on_load(true);
		dataPtr = stbi_loadf(
			filename.c_str(),
			reinterpret_cast<int*>(&m_currentTextureData.m_header.width),
			reinterpret_cast<int*>(&m_currentTextureData.m_header.height), &texChannels, STBI_rgb_alpha);

		m_currentTextureData.m_header.format = RGBA32;

		dataSize = m_currentTextureData.m_header.width * m_currentTextureData.m_header.height * 4 * 4;
	}
	else
	{
		dataPtr = stbi_load(
			filename.c_str(),
			reinterpret_cast<int*>(&m_currentTextureData.m_header.width),
			reinterpret_cast<int*>(&m_currentTextureData.m_header.height), &texChannels, STBI_rgb_alpha);

		m_currentTextureData.m_header.format = RGBA8;

		dataSize = m_currentTextureData.m_header.width * m_currentTextureData.m_header.height * 4;
	}
	if (dataPtr == nullptr) return false;

	m_currentTextureData.m_data.resize(dataSize);
	memcpy(m_currentTextureData.m_data.data(), dataPtr, dataSize);
	stbi_image_free(dataPtr);
	return true;
}

bool TextureLoader::WriteData(const std::string& dataFilename, std::string& errorMessage) const
{
	std::ofstream modelFileStream(dataFilename, std::ofstream::binary | std::ofstream::trunc);

	modelFileStream.write(reinterpret_cast<const char*>(&m_currentTextureData.m_header), sizeof(TextureAssetHeader));
	modelFileStream.write(reinterpret_cast<const char*>(m_currentTextureData.m_data.data()), m_currentTextureData.m_data.size());

	if (modelFileStream.is_open())
	{
		modelFileStream.close();
	}

	return true;
}
