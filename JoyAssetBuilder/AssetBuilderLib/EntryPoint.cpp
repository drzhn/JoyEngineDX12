#include <iostream>

#include <Blob.h>

#include <vector>

#include "ModelConverter.h"
#include "TextureLoader.h"

std::string errorMessage;

JoyEngine::ModelConverter* modelLoader = nullptr;
TextureLoader* textureLoader = nullptr;

extern "C" __declspec(dllexport) int __cdecl InitializeBuilder()
{
	modelLoader = new JoyEngine::ModelConverter();
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
	const char* dataDir,
	const char** errorMessageCStr)
{
	const std::string modelFilename = std::string(modelFileName);
	const std::string dataDirString = std::string(dataDir);
	const std::string dataFilename = std::string(modelFileName) + ".data";

	try
	{
		if (!modelLoader->ConvertModel(modelFilename, dataDirString, errorMessage))
		{
			*errorMessageCStr = errorMessage.c_str();
			return 1;
		}
	}
	catch (const std::exception& e)
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
