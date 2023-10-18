#ifndef UTILS_H
#define UTILS_H

#include <fstream>

[[nodiscard]] inline bool getFileStream(std::ifstream& stream, const std::string& filename, std::string errorMessage)
{
	stream.open(filename);
	if (!stream.is_open())
	{
		errorMessage = "Cannot open stream";
		return false;
	}
	return true;
}
#endif // UTILS_H
