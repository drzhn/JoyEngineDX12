#include "ModelLoader.h"

#include <filesystem>
#include <iostream>
#include <JoyAssetHeaders.h>
#include <map>

#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>

#include "Utils.h"

const char* props[22] = {
	FbxSurfaceMaterial::sShadingModel,
	FbxSurfaceMaterial::sMultiLayer,
	FbxSurfaceMaterial::sEmissive,
	FbxSurfaceMaterial::sEmissiveFactor,
	FbxSurfaceMaterial::sAmbient,
	FbxSurfaceMaterial::sAmbientFactor,
	FbxSurfaceMaterial::sDiffuse,
	FbxSurfaceMaterial::sDiffuseFactor,
	FbxSurfaceMaterial::sSpecular,
	FbxSurfaceMaterial::sSpecularFactor,
	FbxSurfaceMaterial::sShininess,
	FbxSurfaceMaterial::sBump,
	FbxSurfaceMaterial::sNormalMap,
	FbxSurfaceMaterial::sBumpFactor,
	FbxSurfaceMaterial::sTransparentColor,
	FbxSurfaceMaterial::sTransparencyFactor,
	FbxSurfaceMaterial::sReflection,
	FbxSurfaceMaterial::sReflectionFactor,
	FbxSurfaceMaterial::sDisplacementColor,
	FbxSurfaceMaterial::sDisplacementFactor,
	FbxSurfaceMaterial::sVectorDisplacementColor,
	FbxSurfaceMaterial::sVectorDisplacementFactor,
};

// Simple data for nodes without hierarchy
struct NodeData
{
	FbxMesh* mesh = nullptr;
	FbxSurfaceMaterial* material = nullptr;
	std::string materialName;
};

void FindData(FbxNode* node, std::vector<NodeData>& nodes)
{
	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
		if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			nodes.push_back({
				.mesh = static_cast<FbxMesh*>(attr),
				.material = node->GetMaterial(0),
			});
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		FindData(node->GetChild(i), nodes);
	}
}


ModelLoader::ModelLoader()
{
	m_lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(m_lSdkManager, IOSROOT);
	m_lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	m_lImporter = FbxImporter::Create(m_lSdkManager, "");

	m_lGeometryConverter = std::make_unique<FbxGeometryConverter>(m_lSdkManager);
}

ModelLoader::~ModelLoader()
{
	m_lImporter->Destroy();
	// Destroy the SDK manager and all the other objects it was handling.
	m_lSdkManager->Destroy();
}

bool ModelLoader::LoadModel(const std::string& modelFilename, const std::string& dataDir, std::string& errorMessage)
{
	std::filesystem::path modelDirectory = std::filesystem::path(modelFilename).parent_path();
	std::filesystem::path dataDirectory = std::filesystem::path(dataDir);

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

	FbxNode* lRootNode = lScene->GetRootNode();

	std::vector<NodeData> nodes;

	FindData(lRootNode, nodes);

	struct materialdata
	{
		uint32_t materialIndex;
		std::string diffuseTexture;
	};
	std::map<std::string, materialdata> materialIndices;
	uint32_t currentMaterialIndex = 0;

	for (auto& node : nodes)
	{
		FbxSurfaceMaterial* material = node.material;
		auto shadingModel = material->ShadingModel.Get();
		//FbxSurfacePhong* phong = static_cast<FbxSurfacePhong*>(material);
		//auto name = phong->GetName();

		std::string materialName = material->GetName();

		std::cout << materialName << " " << shadingModel << std::endl;

		if (!materialIndices.contains(materialName))
		{
			FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
			//int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			const int textureIndex = 0;
			FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(textureIndex));
			std::string diffuseTexture;
			if (texture != nullptr)
			{
				// fbx stores absolute path in the PC the file created
				std::filesystem::path textureAbsolutePath = texture->GetFileName();
				diffuseTexture = std::filesystem::relative(
					modelDirectory / textureAbsolutePath.filename(),
					dataDirectory
				).generic_string();
			}

			materialIndices.insert({materialName, {currentMaterialIndex, diffuseTexture}});
			currentMaterialIndex++;


			//for (int i = 0; i < 22; i++)
			//{
			//	FbxSurfacePhong* phong = static_cast<FbxSurfacePhong*>(material);
			//	FbxProperty prop1 = phong->FindProperty(props[i]);

			//	FbxFileTexture* texture1 = FbxCast<FbxFileTexture>(prop1.GetSrcObject<FbxTexture>(0));
			//	if (texture1 != nullptr)
			//	{
			//		std::cout << props[i] << " " << texture1->GetFileName() << std::endl;
			//	}
			//}
		}

		node.materialName = materialName;
	}

	m_shapes.resize(nodes.size());

	for (int nodesIndex = 0; nodesIndex < nodes.size(); nodesIndex++)
	{
		auto mesh = nodes[nodesIndex].mesh;

		mesh = static_cast<FbxMesh*>(m_lGeometryConverter->Triangulate(mesh, true));
		const auto polygonsCount = mesh->GetPolygonCount();
		FbxStringList uvSetNames;
		mesh->GetUVSetNames(uvSetNames);

		mesh->GenerateTangentsData(uvSetNames[0]);
		auto tangentsCount = mesh->GetElementTangentCount();
		FbxGeometryElementTangent* tangents = mesh->GetElementTangent(0);
		FbxLayerElementArrayTemplate<FbxVector4>& directArray = tangents->GetDirectArray();
		FbxLayerElementArrayTemplate<int>& indexArray = tangents->GetIndexArray();

		m_shapes[nodesIndex].m_vertices = std::vector<Vertex>(polygonsCount * 3);
		m_shapes[nodesIndex].m_indices = std::vector<Index>(polygonsCount * 3);
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

				FbxVector4 tangent;
				switch (tangents->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					tangent = directArray.GetAt(index);
					break;
				case FbxGeometryElement::eIndexToDirect:
					{
						int id = indexArray.GetAt(index);
						tangent = directArray.GetAt(id);
					}
					break;
				default:
					break; // other reference modes not shown here!
				}

				m_shapes[nodesIndex].m_vertices[i * 3 + vi] = Vertex{
					.pos = {
						static_cast<float>(position[0]),
						static_cast<float>(position[1]),
						static_cast<float>(position[2])
					},
					.normal = {
						static_cast<float>(normal[0]),
						static_cast<float>(normal[1]),
						static_cast<float>(normal[2])
					},
					.tangent = {
						static_cast<float>(tangent[0]),
						static_cast<float>(tangent[1]),
						static_cast<float>(tangent[2])
					},
					.texCoord = {
						static_cast<float>(uv[0]),
						1.f - static_cast<float>(uv[1])
					}
				};
				m_shapes[nodesIndex].m_indices[i * 3 + vi] = i * 3 + vi;
			}
		}

		m_shapes[nodesIndex].m_header = {
			.vertexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Vertex)),
			.indexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Index)),
			.materialIndex = materialIndices.at(nodes[nodesIndex].materialName).materialIndex
		};
	}

	lScene->Destroy(true);

	if (nodes.size() == 1) return true;

	std::vector<std::string> materials(currentMaterialIndex);
	for (const auto& value : materialIndices)
	{
		materials[value.second.materialIndex] = value.first;
	}

	std::filesystem::path materialPath = std::filesystem::path(modelFilename).replace_extension(".json");

	rapidjson::Document json;
	json.SetObject();

	rapidjson::Document::AllocatorType& alloc = json.GetAllocator();


	json.AddMember("type", "standard_material_list", alloc);

	rapidjson::Value mat(rapidjson::kArrayType);
	for (int i = 0; i < currentMaterialIndex; i++)
	{
		rapidjson::Value m(rapidjson::kObjectType);
		m.AddMember("name", rapidjson::Value(materials[i].c_str(), alloc), alloc);
		m.AddMember("diffuse", rapidjson::Value(materialIndices[materials[i]].diffuseTexture.c_str(), alloc), alloc);
		m.AddMember("textureSampler", "linearWrap", alloc);

		mat.PushBack(m, alloc);
	}
	json.AddMember("materials", mat, alloc);

	std::ofstream materialStream(materialPath, std::ofstream::trunc);

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	json.Accept(writer);

	materialStream.write(buffer.GetString(), buffer.GetSize());

	materialStream.close();

	return true;
}

bool ModelLoader::WriteData(const std::string& dataFilename, std::string& errorMessage) const
{
	std::ofstream modelFileStream(dataFilename, std::ofstream::binary | std::ofstream::trunc);

	for (const auto& shape : m_shapes)
	{
		modelFileStream.write(reinterpret_cast<const char*>(&shape.m_header), sizeof(JoyEngine::MeshAssetHeader));
		modelFileStream.write(reinterpret_cast<const char*>(shape.m_vertices.data()), shape.m_header.vertexDataSize);
		modelFileStream.write(reinterpret_cast<const char*>(shape.m_indices.data()), shape.m_header.indexDataSize);
	}

	if (modelFileStream.is_open())
	{
		modelFileStream.close();
	}

	return true;
}
