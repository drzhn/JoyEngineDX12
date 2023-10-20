#include "ModelLoader.h"

#include <filesystem>
#include <iostream>
#include <JoyAssetHeaders.h>
#include <map>

#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>

#include "Utils.h"

#define PROPS_COUNT 14

const char* props[PROPS_COUNT] = {
	// lambert
	FbxSurfaceMaterial::sShadingModel,
	FbxSurfaceMaterial::sDiffuse,
	FbxSurfaceMaterial::sEmissive,
	FbxSurfaceMaterial::sEmissiveFactor,
	FbxSurfaceMaterial::sAmbient,
	FbxSurfaceMaterial::sAmbientFactor,
	FbxSurfaceMaterial::sNormalMap,

	// + alpha cutoff
	FbxSurfaceMaterial::sTransparentColor,
	// + transparent
	FbxSurfaceMaterial::sTransparencyFactor,

	// + phong
	FbxSurfaceMaterial::sSpecular,
	FbxSurfaceMaterial::sSpecularFactor,
	FbxSurfaceMaterial::sReflection,
	FbxSurfaceMaterial::sReflectionFactor,
	FbxSurfaceMaterial::sShininess,

	//FbxSurfaceMaterial::sMultiLayer,
	//FbxSurfaceMaterial::sDiffuseFactor,
	//FbxSurfaceMaterial::sBump,
	//FbxSurfaceMaterial::sBumpFactor,

	//FbxSurfaceMaterial::sDisplacementColor,
	//FbxSurfaceMaterial::sDisplacementFactor,
	//FbxSurfaceMaterial::sVectorDisplacementColor,
	//FbxSurfaceMaterial::sVectorDisplacementFactor,
};

// Simple data for nodes without hierarchy
struct NodeData
{
	FbxMesh* mesh = nullptr;
	FbxSurfaceMaterial* material = nullptr;
	std::string materialName;
};

struct MaterialData
{
	uint32_t materialIndex;

	std::string DiffuseMap;
	std::string EmissiveMap;
	float EmissiveFactor;
	std::string AmbientMap;
	float AmbientFactor;
	std::string NormalMap;
	std::string TransparentColor;
	float TransparencyFactor;
	std::string SpecularMap;
	float SpecularFactor;
	std::string ReflectionMap;
	float ReflectionFactor;
	std::string ShininessMap;
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


	std::map<std::string, MaterialData> materialIndices;
	uint32_t currentMaterialIndex = 0;

	for (auto& node : nodes)
	{
		FbxSurfaceMaterial* material = node.material;
		auto shadingModel = material->ShadingModel.Get();

		std::string materialName = material->GetName();

		std::cout << materialName << " " << shadingModel << std::endl;

		if (!materialIndices.contains(materialName))
		{
			auto GetFloatByProperty = [&](const char* property)
			{
				FbxProperty prop = material->FindProperty(property);
				if (prop.IsValid())
				{
					return static_cast<float>(prop.Get<FbxDouble>());
				}
				return 0.f;
			};

			auto GetTextureNameByProperty = [&](const char* property)
			{
				FbxProperty prop = material->FindProperty(property);
				int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
				int textureCount = prop.GetSrcObjectCount<FbxTexture>();
				if (layeredTextureCount != 0 || textureCount != 0)
				{
					// TODO we dont have multiple textures support yet
				}
				FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(0));
				std::string textureName;
				if (texture != nullptr)
				{
					// fbx stores absolute path in the PC the file created
					std::filesystem::path textureAbsolutePath = texture->GetFileName();
					textureName = std::filesystem::relative(
						modelDirectory / textureAbsolutePath.filename(),
						dataDirectory
					).generic_string();
				}
				return textureName;
			};


			materialIndices.insert({
				materialName, {
					.materialIndex = currentMaterialIndex,
					.DiffuseMap = GetTextureNameByProperty(FbxSurfaceMaterial::sDiffuse),
					.EmissiveMap = GetTextureNameByProperty(FbxSurfaceMaterial::sEmissive),
					.EmissiveFactor = GetFloatByProperty(FbxSurfaceMaterial::sEmissiveFactor),
					.AmbientMap = GetTextureNameByProperty(FbxSurfaceMaterial::sAmbient),
					.AmbientFactor = GetFloatByProperty(FbxSurfaceMaterial::sAmbientFactor),
					.NormalMap = GetTextureNameByProperty(FbxSurfaceMaterial::sNormalMap),
					.TransparentColor = GetTextureNameByProperty(FbxSurfaceMaterial::sTransparentColor),
					.TransparencyFactor = GetFloatByProperty(FbxSurfaceMaterial::sTransparencyFactor),
					.SpecularMap = GetTextureNameByProperty(FbxSurfaceMaterial::sSpecular),
					.SpecularFactor = GetFloatByProperty(FbxSurfaceMaterial::sSpecularFactor),
					.ReflectionMap = GetTextureNameByProperty(FbxSurfaceMaterial::sReflection),
					.ReflectionFactor = GetFloatByProperty(FbxSurfaceMaterial::sReflectionFactor),
					.ShininessMap = GetTextureNameByProperty(FbxSurfaceMaterial::sShininess),
				}
			});
			currentMaterialIndex++;


			for (int i = 0; i < PROPS_COUNT; i++)
			{
				FbxSurfacePhong* phong = static_cast<FbxSurfacePhong*>(material);
				FbxProperty prop1 = phong->FindProperty(props[i]);

				FbxFileTexture* texture1 = FbxCast<FbxFileTexture>(prop1.GetSrcObject<FbxTexture>(0));
				if (texture1 != nullptr)
				{
					std::cout << props[i] << " " << texture1->GetFileName() << std::endl;
				}
			}
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


	json.AddMember("asset_type", "standard_material_list", alloc);

	rapidjson::Value jsonMaterialArray(rapidjson::kArrayType);
	for (int i = 0; i < currentMaterialIndex; i++)
	{
		std::string materialTypeStr;
		if (materialIndices[materials[i]].TransparencyFactor != 0)
		{
			materialTypeStr = "standard_transparent";
		}
		else
		{
			if (materialIndices[materials[i]].TransparentColor.empty())
			{
				materialTypeStr = "standard";
			}
			else
			{
				materialTypeStr = "standard_cutoff";
			}
		}

		rapidjson::Value jsonMaterialEntry(rapidjson::kObjectType);
		jsonMaterialEntry.AddMember("asset_type", rapidjson::Value("material", alloc), alloc);
		jsonMaterialEntry.AddMember("name", rapidjson::Value(materials[i].c_str(), alloc), alloc);
		jsonMaterialEntry.AddMember("material_type", rapidjson::Value(materialTypeStr.c_str(), alloc), alloc);

		{
			rapidjson::Value jsonBindingsEntry(rapidjson::kObjectType);
			jsonBindingsEntry.AddMember("DiffuseMap", rapidjson::Value(materialIndices[materials[i]].DiffuseMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("EmissiveMap", rapidjson::Value(materialIndices[materials[i]].EmissiveMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("EmissiveFactor", rapidjson::Value(materialIndices[materials[i]].EmissiveFactor), alloc);
			jsonBindingsEntry.AddMember("AmbientMap", rapidjson::Value(materialIndices[materials[i]].AmbientMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("AmbientFactor", rapidjson::Value(materialIndices[materials[i]].AmbientFactor), alloc);
			jsonBindingsEntry.AddMember("NormalMap", rapidjson::Value(materialIndices[materials[i]].NormalMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("TransparentColor", rapidjson::Value(materialIndices[materials[i]].TransparentColor.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("TransparencyFactor", rapidjson::Value(materialIndices[materials[i]].TransparencyFactor), alloc);
			jsonBindingsEntry.AddMember("SpecularMap", rapidjson::Value(materialIndices[materials[i]].SpecularMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("SpecularFactor", rapidjson::Value(materialIndices[materials[i]].SpecularFactor), alloc);
			jsonBindingsEntry.AddMember("ReflectionMap", rapidjson::Value(materialIndices[materials[i]].ReflectionMap.c_str(), alloc), alloc);
			jsonBindingsEntry.AddMember("ReflectionFactor", rapidjson::Value(materialIndices[materials[i]].ReflectionFactor), alloc);
			jsonBindingsEntry.AddMember("ShininessMap", rapidjson::Value(materialIndices[materials[i]].ShininessMap.c_str(), alloc), alloc);

			jsonBindingsEntry.AddMember("TextureSampler", "linearWrap", alloc);

			jsonMaterialEntry.AddMember("bindings", jsonBindingsEntry, alloc);
		}

		jsonMaterialArray.PushBack(jsonMaterialEntry, alloc);
	}
	json.AddMember("materials", jsonMaterialArray, alloc);

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
