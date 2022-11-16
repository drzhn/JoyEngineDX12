#include "TextureLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <filesystem>

#include "stb_image.h"

#include <fstream>
#include <iostream>

std::string exec(const char* cmd)
{
	char buffer[128];
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer, 128, pipe.get()) != nullptr)
	{
		result += buffer;
	}
	return result;
}

bool TextureLoader::LoadTexture(const std::string& filePath, std::string& errorMessage)
{
	int texChannels;
	const bool isHdr = stbi_is_hdr(filePath.c_str());

	size_t dataSize = 0;

	std::filesystem::path fullPath = std::filesystem::path(filePath);
	std::filesystem::path folder = fullPath.parent_path();
	std::filesystem::path ddsfilename = fullPath.stem().generic_string() + ".dds";
	std::filesystem::path generatedFile = folder / ddsfilename;

	std::string command;
	if (isHdr)
	{
		m_currentTextureData.m_header.format = BC6H_UF16;
		command = "texconv.exe " + fullPath.generic_string() + " -m 0 -f BC6H_UF16 -y";
	}
	else
	{
		m_currentTextureData.m_header.format = BC1_UNORM;
		command = "texconv.exe " + fullPath.generic_string() + " -m 0 -f BC1_UNORM -y";
	}

	std::string output = exec(command.c_str());
	std::cout << output;

	std::ifstream file(generatedFile, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		errorMessage = "Couldn't open generated file " + generatedFile.generic_string();
		return false;
	}

	dataSize = file.tellg();
	file.seekg(0, std::ios::beg);

	m_currentTextureData.m_data.resize(dataSize);
	if (!file.read(m_currentTextureData.m_data.data(), dataSize))
	{
		errorMessage = "Couldn't read generated file " + generatedFile.generic_string();
		return false;
	}
	m_currentTextureData.m_header.dataSize = dataSize;

	auto openBraceIndex = output.find_last_of('(');
	auto closeBraceIndex = output.find_last_of(')');

	auto info = output.substr(openBraceIndex + 1, closeBraceIndex - openBraceIndex - 1);

	auto xpos = info.find_first_of('x');
	auto commaPos = info.find_first_of(',');
	auto spacePos = info.find_first_of(' ');

	m_currentTextureData.m_header.width = std::stoul(info.substr(0, xpos));
	m_currentTextureData.m_header.height = std::stoul(info.substr(xpos + 1, commaPos - xpos));
	m_currentTextureData.m_header.mipCount = std::stoul(info.substr(commaPos + 1, spacePos - commaPos));
	file.close();

	if (!std::filesystem::remove(generatedFile))
	{
		errorMessage = "Couldn't delete generated file " + generatedFile.generic_string();
		return false;
	}

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
