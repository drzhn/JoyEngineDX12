#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "JoyAssetHeaders.h"
#include "tiny_obj_loader.h"
#include <stdexcept>
#include <fstream>
#include <vector>

class ModelLoader
{
public:
	[[nodiscard]]
	static bool getFileStream(std::ifstream& stream, const std::string& filename, std::string errorMessage)
	{
		stream.open(filename);
		if (!stream.is_open())
		{
			errorMessage = "Cannot open stream";
			return false;
		}
		return true;
	}

	[[nodiscard]]
	static bool LoadModel(std::vector<Vertex>& vertices,
	                      std::vector<uint32_t>& indices,
	                      const std::string& filename,
	                      std::string& errorMessage)
	{
		bool res;
		std::ifstream stream;
		res = getFileStream(stream, filename, errorMessage);
		if (!res)
		{
			return false;
		}

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &stream);
		if (!res)
		{
			errorMessage = err;
			return false;
		}

		uint32_t vertSize = 0;
		for (const auto& shape : shapes)
		{
			vertSize += shape.mesh.indices.size();
		}
		vertices.resize(vertSize);
		indices.resize(vertSize);

		uint32_t vertIndex = 0;
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				vertices[vertIndex].pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertices[vertIndex].normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				vertices[vertIndex].texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertices[vertIndex].color = {1.0f, 1.0f, 1.0f};

				indices[vertIndex] = vertIndex;
				vertIndex++;
			}
		}
		if (stream.is_open())
		{
			stream.close();
		}

		return true;
	}
};


#endif //MODEL_LOADER_H
