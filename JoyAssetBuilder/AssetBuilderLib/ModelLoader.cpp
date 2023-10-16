#include "ModelLoader.h"

#include <iostream>
#include <JoyAssetHeaders.h>
#include "Utils.h"

FbxMesh* FindMesh(FbxNode* node)
{
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
		if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			return static_cast<FbxMesh*>(attr);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		FbxMesh* mesh = FindMesh(node->GetChild(i));
		if (mesh != nullptr)
		{
			return mesh;
		}
	}

	return nullptr;
}


ModelLoader::ModelLoader()
{
	m_lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(m_lSdkManager, IOSROOT);
	m_lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	m_lImporter = FbxImporter::Create(m_lSdkManager, "");
}

ModelLoader::~ModelLoader()
{
	m_lImporter->Destroy();
	// Destroy the SDK manager and all the other objects it was handling.
	m_lSdkManager->Destroy();
}

bool ModelLoader::LoadModel(const std::string& modelFilename, const std::string& materialsDir, std::string& errorMessage)
{
	// Use the first argument as the filename for the importer.
	if (!m_lImporter->Initialize(modelFilename.c_str(), -1, m_lSdkManager->GetIOSettings()))
	{
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", m_lImporter->GetStatus().GetErrorString());

		errorMessage = m_lImporter->GetStatus().GetErrorString();
		return false;
	}

	// Create a new scene so that it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(m_lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	m_lImporter->Import(lScene);

	//// The file is imported; so get rid of the importer.
	//m_lImporter->Destroy();

	FbxNode* lRootNode = lScene->GetRootNode();

	FbxMesh* mesh = FindMesh(lRootNode);

	FbxGeometryConverter lGeometryConverter(m_lSdkManager);

	if (mesh != nullptr)
	{
		m_shapes.resize(1);

		mesh = static_cast<FbxMesh*>(lGeometryConverter.Triangulate(mesh, true));

		const auto polygonsCount = mesh->GetPolygonCount();
		//const auto vertexCount = mesh->GetPolygonVertexCount();
		//FbxArray<FbxVector4> pNormals;
		//bool result = mesh->GetPolygonVertexNormals(pNormals);
		FbxStringList uvSetNames;
		mesh->GetUVSetNames(uvSetNames);

		//int uvCount = mesh->GetUVLayerCount();
		//FbxArray<FbxVector2> pUVs;
		//result = mesh->GetPolygonVertexUVs(uvSetNames.GetStringAt(0), pUVs);

		m_shapes[0].m_vertices = std::vector<Vertex>(polygonsCount * 3);
		m_shapes[0].m_indices = std::vector<Index>(polygonsCount * 3);
		for (int i = 0; i < polygonsCount; i++)
		{
			for (int vi = 0; vi < 3; vi++)
			{
				int index = mesh->GetPolygonVertex(i, vi);
				FbxVector4 position = mesh->GetControlPointAt(index);
				FbxVector4 normal;
				mesh->GetPolygonVertexNormal(i, vi, normal);
				FbxVector2 uv;
				bool unmapped;
				mesh->GetPolygonVertexUV(i, vi, uvSetNames.GetStringAt(0), uv, unmapped);

				m_shapes[0].m_vertices[i * 3 + vi] = Vertex{
					.pos = {
						static_cast<float>(position[0]),
						static_cast<float>(position[1]),
						static_cast<float>(position[2])
					},
					.color = {},
					.normal = {
						static_cast<float>(normal[0]),
						static_cast<float>(normal[1]),
						static_cast<float>(normal[2])
					},
					.texCoord = {
						static_cast<float>(uv[0]),
						1.f - static_cast<float>(uv[1])
					}
				};
				m_shapes[0].m_indices[i * 3 + vi] = i * 3 + vi;
			}
		}

		m_shapes[0].m_header = {
			.vertexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Vertex)),
			.indexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Index)),
			.materialIndex = 0
		};
	}

	lScene->Destroy(true);
	//// Destroy the SDK manager and all the other objects it was handling.
	//m_lSdkManager->Destroy();

	//bool res;
	//std::ifstream stream;
	//res = getFileStream(stream, modelFilename, errorMessage);
	//if (!res)
	//{
	//	return false;
	//}

	//tinyobj::attrib_t attrib;
	//std::vector<tinyobj::shape_t> shapes;
	//std::vector<tinyobj::material_t> materials;
	//std::string warn, err;

	//res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelFilename.c_str(), 
	//	materialsDir.c_str());
	//if (!res)
	//{
	//	errorMessage = err;
	//	return false;
	//}

	//m_shapes.resize(shapes.size()); 

	//for (const auto& shape : shapes)
	//{
	//	std::cout << shape.mesh.material_ids[0] << " ";
	//}

	//for (uint32_t i = 0; i < shapes.size(); i++)
	//{
	//	uint32_t vertSize = shapes[i].mesh.indices.size();
	//	m_shapes[i].m_header.vertexDataSize = vertSize * sizeof(Vertex); // wtf??
	//	m_shapes[i].m_header.indexDataSize = vertSize * sizeof(uint32_t);
	//	m_shapes[i].m_header.materialIndex = shapes[i].mesh.material_ids[0]; // we expect that every face in mesh has the same mat id

	//	m_shapes[i].m_vertices.resize(vertSize);
	//	m_shapes[i].m_indices.resize(vertSize);
	//}


	//for (uint32_t i = 0; i < shapes.size(); i++)
	//{
	//	uint32_t vertIndex = 0;
	//	for (const auto& index : shapes[i].mesh.indices)
	//	{
	//		m_shapes[i].m_vertices[vertIndex].pos = {
	//			attrib.vertices[3 * index.vertex_index + 0],
	//			attrib.vertices[3 * index.vertex_index + 1],
	//			attrib.vertices[3 * index.vertex_index + 2]
	//		};
	//		m_shapes[i].m_vertices[vertIndex].normal = {
	//			attrib.normals[3 * index.normal_index + 0],
	//			attrib.normals[3 * index.normal_index + 1],
	//			attrib.normals[3 * index.normal_index + 2]
	//		};
	//		m_shapes[i].m_vertices[vertIndex].texCoord = {
	//			attrib.texcoords[2 * index.texcoord_index + 0],
	//			1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
	//		};

	//		m_shapes[i].m_vertices[vertIndex].color = {1.0f, 1.0f, 1.0f};

	//		m_shapes[i].m_indices[vertIndex] = vertIndex;
	//		vertIndex++;
	//	}
	//}

	//if (stream.is_open())
	//{
	//	stream.close();
	//}

	return true;
}

bool ModelLoader::WriteData(const std::string& dataFilename, std::string& errorMessage) const
{
	std::ofstream modelFileStream(dataFilename, std::ofstream::binary | std::ofstream::trunc);

	for (const auto& shape : m_shapes)
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
