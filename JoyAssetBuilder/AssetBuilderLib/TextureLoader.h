#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#define STB_IMAGE_IMPLEMENTATION
#include <string>
#include <vector>

#include "stb_image.h"

#include "JoyAssetHeaders.h"

class TextureLoader
{
public:
	[[nodiscard]]
	static bool LoadTexture(const std::string& filename, std::vector<unsigned char>& data,
		uint32_t* width,
		uint32_t* height,
		TextureFormat* type)
	{
		int texChannels;
		bool isHdr = stbi_is_hdr(filename.c_str());

		//if (isHdr)
		//{
		//	stbi_set_flip_vertically_on_load(true);
		//	float* dataPtr = stbi_loadf(
		//		filename.c_str(),
		//		reinterpret_cast<int*>(width),
		//		reinterpret_cast<int*>(height), &texChannels, STBI_rgb_alpha);
		//	if (dataPtr == nullptr)
		//	{
		//		return false;
		//	}
		//	*type = RGB_FLOAT;

		//	size_t dataSize = (*width) * (*height) * 4 * 4;
		//	data.resize(dataSize);
		//	memcpy(data.data(), dataPtr, dataSize);
		//	delete[] dataPtr;
		//	return true;
		//}
		//else
		{
			unsigned char* dataPtr = stbi_load(
				filename.c_str(),
				reinterpret_cast<int*>(width),
				reinterpret_cast<int*>(height), &texChannels, STBI_rgb_alpha);
			if (dataPtr == nullptr)
			{
				return false;
			}
			*type = RGBA8;

			size_t dataSize = (*width) * (*height) * 4;
			data.resize(dataSize);
			memcpy(data.data(), dataPtr, dataSize);
			delete[] dataPtr;
			return true;
		}
	}
};

#endif //TEXTURE_LOADER_H
