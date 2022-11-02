#include "ModelLoader.h"

#include <iostream>
#include <JoyAssetHeaders.h>
#include <tiny_obj_loader.h>

#include "Utils.h"

bool ModelLoader::LoadModel(const std::string& modelFilename, const std::string& materialsDir, std::string& errorMessage)
{
	bool res;
	std::ifstream stream;
	res = getFileStream(stream, modelFilename, errorMessage);
	if (!res)
	{
		return false;
	}

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelFilename.c_str(), 
		materialsDir.c_str());
	if (!res)
	{
		errorMessage = err;
		return false;
	}

	m_shapes.resize(shapes.size()); 

	for (const auto& shape : shapes)
	{
		std::cout << shape.mesh.material_ids[0] << " ";
	}

	for (uint32_t i = 0; i < shapes.size(); i++)
	{
		uint32_t vertSize = shapes[i].mesh.indices.size();
		m_shapes[i].m_header.vertexDataSize = vertSize * sizeof(Vertex); // wtf??
		m_shapes[i].m_header.indexDataSize = vertSize * sizeof(uint32_t);
		m_shapes[i].m_header.materialIndex = shapes[i].mesh.material_ids[0]; // we expect that every face in mesh has the same mat id

		m_shapes[i].m_vertices.resize(vertSize);
		m_shapes[i].m_indices.resize(vertSize);
	}


	for (uint32_t i = 0; i < shapes.size(); i++)
	{
		uint32_t vertIndex = 0;
		for (const auto& index : shapes[i].mesh.indices)
		{
			m_shapes[i].m_vertices[vertIndex].pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			m_shapes[i].m_vertices[vertIndex].normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};
			m_shapes[i].m_vertices[vertIndex].texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			m_shapes[i].m_vertices[vertIndex].color = {1.0f, 1.0f, 1.0f};

			m_shapes[i].m_indices[vertIndex] = vertIndex;
			vertIndex++;
		}
	}

	if (stream.is_open())
	{
		stream.close();
	}

	return true;
}

bool ModelLoader::WriteData(const std::string& dataFilename, std::string& errorMessage) const
{
	std::ofstream modelFileStream(dataFilename, std::ofstream::binary | std::ofstream::trunc);

	for (const auto & shape : m_shapes)
	{
		modelFileStream.write(reinterpret_cast<const char*>(&shape.m_header), sizeof(MeshAssetHeader));
		modelFileStream.write(reinterpret_cast<const char*>(shape.m_vertices.data()), shape.m_header.vertexDataSize);
		modelFileStream.write(reinterpret_cast<const char*>(shape.m_indices.data()), shape.m_header.indexDataSize);
	}

	if (modelFileStream.is_open())
	{
		modelFileStream.close();
	}

	return true;
}
