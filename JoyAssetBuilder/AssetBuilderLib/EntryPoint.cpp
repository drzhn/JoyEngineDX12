#include <iostream>
#include <vector>

#include "ModelLoader.h"
#include "TextureLoader.h"

extern "C" __declspec(dllexport) int __cdecl InitializeBuilder()
{
	std::cout << "Builder initialized" << std::endl;
	return 0;
}

extern "C" __declspec(dllexport) int __cdecl TerminateBuilder()
{
	std::cout << "Builder terminated" << std::endl;
	return 0;
}

std::string errorMessage;

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
std::vector<unsigned char> textureData;

extern "C" __declspec(dllexport) int __cdecl BuildModel(
	const char* modelFileName,
	const void** vertexDataPtr,
	unsigned long long* vertexDataSize,
	const void** indexDataPtr,
	unsigned long long* indexDataSize,
	const char** errorMessageCStr)
{
	const std::string filename = std::string(modelFileName);
	bool res = ModelLoader::LoadModel(vertices, indices, filename, errorMessage);
	if (!res)
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}

	*vertexDataPtr = vertices.data();
	*vertexDataSize = vertices.size() * sizeof(Vertex);
	*indexDataPtr = indices.data();
	*indexDataSize = indices.size() * sizeof(uint32_t);
	return 0;
}

extern "C" __declspec(dllexport) int __cdecl BuildTexture(
	const char* textureFileName,
	const void** textureDataPtr,
	unsigned long long* textureDataSize,
	uint32_t * textureWidth,
	uint32_t * textureHeight,
	uint32_t * textureType,
	const char** errorMessageCStr)
{
	const std::string filename = std::string(textureFileName);
	bool res = TextureLoader::LoadTexture(
		filename,
		textureData,
		textureWidth,
		textureHeight,
		reinterpret_cast<TextureFormat*>(textureType));
	if (!res)
	{
		*errorMessageCStr = errorMessage.c_str();
		return 1;
	}
	*textureDataPtr = textureData.data();
	*textureDataSize = textureData.size();

	return 0;
}
