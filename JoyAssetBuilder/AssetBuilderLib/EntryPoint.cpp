#include <iostream>
#include <vector>

#include "ModelLoader.h"
#include "TextureLoader.h"

ModelLoader* modelLoader = nullptr;

extern "C" __declspec(dllexport) int __cdecl InitializeBuilder()
{
	modelLoader = new ModelLoader();

	std::cout << "Builder initialized" << std::endl;
	return 0;
}

extern "C" __declspec(dllexport) int __cdecl TerminateBuilder()
{
	delete modelLoader;

	std::cout << "Builder terminated" << std::endl;
	return 0;
}

std::string errorMessage;

std::vector<unsigned char> textureData;

extern "C" __declspec(dllexport) int __cdecl BuildModel(
	const char* modelFileName,
	const char* materialsDir,
	const char** errorMessageCStr)
{
	const std::string modelFilename = std::string(modelFileName);
	const std::string materialsDirString = std::string(materialsDir);
	const std::string dataFilename = std::string(modelFileName) + ".data";

	if (!modelLoader->LoadModel(modelFilename, materialsDirString, errorMessage))
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	if (!modelLoader->WriteData(dataFilename, errorMessage))
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	return 0;
}

extern "C" __declspec(dllexport) int __cdecl BuildTexture(
	const char* textureFileName,
	const void** textureDataPtr,
	unsigned long long* textureDataSize,
	uint32_t* textureWidth,
	uint32_t* textureHeight,
	uint32_t* textureType,
	const char** errorMessageCStr)
{
	const std::string filename = std::string(textureFileName);
	bool res = TextureLoader::LoadTexture(
		filename,
		textureData,
		textureWidth,
		textureHeight,
		reinterpret_cast<TextureAssetFormat*>(textureType));
	if (!res)
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}
	*textureDataPtr = textureData.data();
	*textureDataSize = textureData.size();

	return 0;
}
