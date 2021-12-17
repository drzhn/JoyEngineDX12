#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <vector>
#include <string>
#include <fstream>
#include <Utils/Assert.h>

namespace JoyEngine
{
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		ASSERT(file.is_open());

		std::streamsize fileSize = file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}

	static std::ifstream GetStream(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary);

		ASSERT(file.is_open());

		file.seekg(0);

		return file;
	}
}

#endif //FILE_UTILS_H
