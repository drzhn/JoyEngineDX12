#include <iostream>

#include <Blob.h>

#include <vector>

#include "ModelLoader.h"
#include "TextureLoader.h"

std::string errorMessage;

ModelLoader* modelLoader = nullptr;
TextureLoader* textureLoader = nullptr;

extern "C" __declspec(dllexport) int __cdecl InitializeBuilder()
{
	modelLoader = new ModelLoader();
	textureLoader = new TextureLoader();

	std::cout << "Builder initialized" << std::endl;
	return 0;
}

extern "C" __declspec(dllexport) int __cdecl TerminateBuilder()
{
	delete modelLoader;

	std::cout << "Builder terminated" << std::endl;
	return 0;
}


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
	const char** errorMessageCStr)
{
	const std::string textureFilename = std::string(textureFileName);
	const std::string dataFilename = textureFilename + ".data";

	if (!textureLoader->LoadTexture(textureFilename, errorMessage))
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	if (!textureLoader->WriteData(dataFilename, errorMessage))
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	return 0;
}
