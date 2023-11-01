#include "ModelConverter.h"

#include <filesystem>
#include <iostream>
#include <JoyAssetHeaders.h>
#include "Common/HashDefs.h"
#include <map>

#include "rapidjson/document.h"
#include <rapidjson/prettywriter.h>

#include "Utils.h"

#define PROPS_COUNT 14
#define MAX_HIERARCHY_DEPTH 32

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

namespace JoyEngine
{
	struct MaterialData
	{
		std::string materialRelativePath;

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

	struct NodeData
	{
		std::string name;
		jmath::vec3 localPosition;
		jmath::vec3 localRotation;
		jmath::vec3 localScale;
		uint32_t childCount;
		uint32_t childStartIndex;
		uint64_t dataFileOffset;

		std::vector<NodeData*> children;

		std::string materialName;

		bool hasMesh = false;
		std::string meshPath;

		ShapeData shape;
	};

	void CreateGameObjectJson(
		const NodeData* data,
		const std::filesystem::path& modelRelativePath,
		const std::map<std::string, MaterialData>& materialData,
		rapidjson::Value& gameObjectEntry,
		rapidjson::Document::AllocatorType& alloc
	)
	{
		gameObjectEntry.AddMember("name", rapidjson::Value(data->name.c_str(), alloc), alloc);
		gameObjectEntry.AddMember("asset_type", rapidjson::Value("game_object", alloc), alloc);
		{
			auto SerializeFloat3 = [&](jmath::vec3 vec)
			{
				rapidjson::Value arr(rapidjson::kArrayType);
				arr.PushBack(rapidjson::Value(vec.x), alloc);
				arr.PushBack(rapidjson::Value(vec.y), alloc);
				arr.PushBack(rapidjson::Value(vec.z), alloc);
				return arr;
			};
			rapidjson::Value transformEntry(rapidjson::kObjectType);
			transformEntry.AddMember("localPosition", SerializeFloat3(data->localPosition), alloc);
			transformEntry.AddMember("localRotation", SerializeFloat3(data->localRotation), alloc);
			transformEntry.AddMember("localScale", SerializeFloat3(data->localScale), alloc);

			gameObjectEntry.AddMember("transform", transformEntry, alloc);
		}

		rapidjson::Value componentsArray(rapidjson::kArrayType);
		if (data->hasMesh)
		{
			rapidjson::Value componentEntry(rapidjson::kObjectType);
			componentEntry.AddMember("asset_type", rapidjson::Value("renderer"), alloc);
			componentEntry.AddMember("model", rapidjson::Value(
				                         (modelRelativePath.generic_string() + ":" + data->meshPath).c_str(), alloc
			                         ), alloc);
			componentEntry.AddMember("material", rapidjson::Value(
				                         materialData.at(data->materialName).materialRelativePath.c_str(), alloc
			                         ), alloc);
			componentEntry.AddMember("static", false, alloc);
			componentsArray.PushBack(componentEntry, alloc);
		}
		gameObjectEntry.AddMember("components", componentsArray, alloc);

		rapidjson::Value childrenArray(rapidjson::kArrayType);
		for (const auto& child : data->children)
		{
			rapidjson::Value childrenEntry(rapidjson::kObjectType);
			CreateGameObjectJson(child, modelRelativePath, materialData, childrenEntry, alloc);
			childrenArray.PushBack(childrenEntry, alloc);
		}
		gameObjectEntry.AddMember("children", childrenArray, alloc);
	};

	void ProcessMaterial(
		const FbxSurfaceMaterial* material,
		const std::string& materialSafeName,
		std::map<std::string, MaterialData>& materialData,
		const std::filesystem::path& modelDirectory,
		const std::filesystem::path& dataDirectory,
		std::string& errorMessage)
	{
		if (!materialData.contains(materialSafeName))
		{
			const auto shadingModel = material->ShadingModel.Get();
			std::cout << materialSafeName << " " << shadingModel << std::endl;

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
				const FbxProperty prop = material->FindProperty(property);
				const int layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
				const int textureCount = prop.GetSrcObjectCount<FbxTexture>();
				if (layeredTextureCount > 1 || textureCount > 1)
				{
					errorMessage = "we dont have multiple textures support yet";
					throw std::exception();
				}
				const FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(0));
				std::string textureName;
				if (texture != nullptr)
				{
					// fbx stores absolute path in the PC the file created
					const std::filesystem::path textureAbsolutePath = texture->GetFileName();
					textureName = std::filesystem::relative(
						modelDirectory / textureAbsolutePath.filename(),
						dataDirectory
					).generic_string();
				}
				return textureName;
			};


			materialData.insert({
				materialSafeName, {
					.materialRelativePath = std::filesystem::relative(
						modelDirectory / (materialSafeName + ".material"),
						dataDirectory).generic_string(),
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

			for (int i = 0; i < PROPS_COUNT; i++)
			{
				FbxProperty prop1 = material->FindProperty(props[i]);

				FbxFileTexture* texture1 = FbxCast<FbxFileTexture>(prop1.GetSrcObject<FbxTexture>(0));
				if (texture1 != nullptr)
				{
					std::cout << props[i] << " " << texture1->GetFileName() << std::endl;
				}
			}
		}
	}


	void ProcessMesh(
		FbxMesh* mesh,
		NodeData& nodeData,
		FbxGeometryConverter* geometryConverter
	)
	{
		mesh = static_cast<FbxMesh*>(geometryConverter->Triangulate(mesh, true));
		const auto polygonsCount = mesh->GetPolygonCount();
		FbxStringList uvSetNames;
		mesh->GetUVSetNames(uvSetNames);

		std::cout << uvSetNames.GetCount() << std::endl;

		mesh->GenerateTangentsData(uvSetNames[0]);
		FbxGeometryElementTangent* tangents = mesh->GetElementTangent(0);
		const FbxLayerElementArrayTemplate<FbxVector4>& directArray = tangents->GetDirectArray();
		const FbxLayerElementArrayTemplate<int>& indexArray = tangents->GetIndexArray();

		nodeData.shape.m_vertices = std::vector<Vertex>(polygonsCount * 3);
		nodeData.shape.m_indices = std::vector<Index>(polygonsCount * 3);

		for (int i = 0; i < polygonsCount; i++)
		{
			for (int vi = 0; vi < 3; vi++)
			{
				const int index = mesh->GetPolygonVertex(i, vi);
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

				nodeData.shape.m_vertices[i * 3 + (3 - vi) % 3] = Vertex{
					.pos = {
						DirectX::PackedVector::XMConvertFloatToHalf(-static_cast<float>(position[0])),
						DirectX::PackedVector::XMConvertFloatToHalf(static_cast<float>(position[1])),
						DirectX::PackedVector::XMConvertFloatToHalf(static_cast<float>(position[2])),
						DirectX::PackedVector::XMConvertFloatToHalf(1)
					},
					.normal = {
						(1 - static_cast<float>(normal[0])) * 0.5f,
						(1 + static_cast<float>(normal[1])) * 0.5f,
						(1 + static_cast<float>(normal[2])) * 0.5f,
						1.0f
					},
					.tangent = {
						(1 - static_cast<float>(tangent[0])) * 0.5f,
						(1 + static_cast<float>(tangent[1])) * 0.5f,
						(1 + static_cast<float>(tangent[2])) * 0.5f,
						1.0f
					},
					.texCoord = {
						static_cast<float>(uv[0]),
						1.f - static_cast<float>(uv[1])
					}
				};
				nodeData.shape.m_indices[i * 3 + vi] = i * 3 + vi;
			}
		}

		nodeData.shape.m_header = {
			.vertexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Vertex)),
			.indexDataSize = static_cast<uint32_t>(polygonsCount * 3 * sizeof(Index)),
		};
	}

	void FindData(
		FbxNode* node,
		std::vector<std::vector<std::unique_ptr<NodeData>>>& nodeDataContainer,
		std::map<std::string, MaterialData>& materialDataContainer,
		const std::filesystem::path& modelDirectory,
		const std::filesystem::path& dataDirectory,
		FbxGeometryConverter* geometryConverter,
		NodeData* parent,
		int hierarchyLevel,
		uint32_t& nodeCount,
		std::string& errorMessage)
	{
		nodeDataContainer[hierarchyLevel].emplace_back(std::move(std::make_unique<NodeData>()));
		NodeData* nodeData = nodeDataContainer[hierarchyLevel].back().get();
		std::cout << node->GetName() << std::endl;
		nodeData->name = node->GetName();
		nodeData->childCount = node->GetChildCount();
		nodeData->localPosition = {
			-static_cast<float>(node->LclTranslation.Get()[0]),
			static_cast<float>(node->LclTranslation.Get()[1]),
			static_cast<float>(node->LclTranslation.Get()[2])
		};
		nodeData->localRotation = {
			static_cast<float>(node->LclRotation.Get()[0]),
			static_cast<float>(node->LclRotation.Get()[1]),
			static_cast<float>(node->LclRotation.Get()[2]),
		};
		nodeData->localScale = {
			static_cast<float>(node->LclScaling.Get()[0]),
			static_cast<float>(node->LclScaling.Get()[1]),
			static_cast<float>(node->LclScaling.Get()[2]),
		};


		if (parent != nullptr)
		{
			parent->children.push_back(nodeData);
		}
		nodeCount++;

		// Material
		{
			//for (int i =0 ; i < node->GetMaterialCount(); i++)
			//{
			//	std::cout << node->GetMaterial(i)->GetName() << " " << node->GetMaterial(i)->ShadingModel.Get() << std::endl;
			//}
			//if (node->GetMaterialCount() > 1)
			//{
			//	// TODO write warning, use first material
			//	errorMessage = "We dont have multiple materials support yet: materials count = " + std::to_string(node->GetMaterialCount());
			//	throw std::exception();
			//}

			if (node->GetMaterialCount() > 0)
			{
				const FbxSurfaceMaterial* materialPtr = node->GetMaterial(0);
				nodeData->materialName = std::string(materialPtr->GetName());
				// TODO check if filename is valid
				std::ranges::replace(nodeData->materialName, ':', '_');
				ProcessMaterial(materialPtr, nodeData->materialName, materialDataContainer, modelDirectory, dataDirectory, errorMessage);
			}
		}

		nodeData->meshPath = (parent == nullptr ? "" : parent->meshPath + "/") + nodeData->name;

		for (int i = 0; i < node->GetNodeAttributeCount(); i++)
		{
			FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);

			if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
			{
				nodeData->hasMesh = true;

				ProcessMesh(static_cast<FbxMesh*>(attr), *nodeData, geometryConverter);
			}
		}


		for (int i = 0; i < node->GetChildCount(); i++)
		{
			FindData(
				node->GetChild(i),
				nodeDataContainer,
				materialDataContainer,
				modelDirectory,
				dataDirectory,
				geometryConverter,
				nodeData,
				hierarchyLevel + 1,
				nodeCount,
				errorMessage);
		}
	}


	ModelConverter::ModelConverter()
	{
		m_lSdkManager = FbxManager::Create();

		// Create the IO settings object.
		FbxIOSettings* ios = FbxIOSettings::Create(m_lSdkManager, IOSROOT);
		m_lSdkManager->SetIOSettings(ios);

		// Create an importer using the SDK manager.
		m_lImporter = FbxImporter::Create(m_lSdkManager, "");

		m_lGeometryConverter = std::make_unique<FbxGeometryConverter>(m_lSdkManager);
	}

	ModelConverter::~ModelConverter()
	{
		m_lImporter->Destroy();
		// Destroy the SDK manager and all the other objects it was handling.
		m_lSdkManager->Destroy();
	}

	bool ModelConverter::ConvertModel(const std::string& modelPath, const std::string& dataDir, std::string& errorMessage)
	{
		std::filesystem::path modelAbsoluteDirectory = std::filesystem::path(modelPath).parent_path();
		std::filesystem::path convertedModelPath = modelPath + ".data";
		std::filesystem::path convertedModelPrefabPath = modelPath + ".prefab";
		std::filesystem::path dataAbsoluteDirectory = std::filesystem::path(dataDir);
		std::filesystem::path modelRelativePath = std::filesystem::relative(modelPath, dataAbsoluteDirectory);

		if (!m_lImporter->Initialize(modelPath.c_str(), -1, m_lSdkManager->GetIOSettings()))
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

		std::map<std::string, MaterialData> materialData;

		// we store hierarchy in rows:
		// 
		// nodeData[0]            root
		//                       /     \
		// nodeData[1]    parent1       parent2
		//                 /   \         /   \
		// nodeData[2] child1 child2 child1 child2

		std::vector<std::vector<std::unique_ptr<NodeData>>> nodeData(MAX_HIERARCHY_DEPTH);
		uint32_t nodeCount = 0;
		FindData(
			lRootNode,
			nodeData,
			materialData,
			modelAbsoluteDirectory,
			dataAbsoluteDirectory,
			m_lGeometryConverter.get(),
			nullptr,
			0,
			nodeCount,
			errorMessage);

		lScene->Destroy(true);

		std::vector<TreeEntry> treeEntries(nodeCount);
		uint64_t currentFileOffset = treeEntries.size() * sizeof(TreeEntry);

		uint32_t nodeCountIncludeThisLevel = 0;
		int currentEntryIndex = 0;

		for (int level = 0; level < MAX_HIERARCHY_DEPTH; level++)
		{
			nodeCountIncludeThisLevel += nodeData[level].size();
			uint32_t thisLevelChildCount = nodeCountIncludeThisLevel;
			for (int i = 0; i < nodeData[level].size(); i++)
			{
				nodeData[level][i]->childStartIndex = thisLevelChildCount;
				thisLevelChildCount += nodeData[level][i]->childCount;

				treeEntries[currentEntryIndex] = {
					.nameHash = StrHash64(nodeData[level][i]->name.c_str()),
					.childCount = nodeData[level][i]->childCount,
					.childStartIndex = nodeData[level][i]->childStartIndex,
					.dataFileOffset = currentFileOffset
				};
				nodeData[level][i]->dataFileOffset = nodeData[level][i]->shape.GetSizeInBytes() == 0 ? INVALID_OFFSET : currentFileOffset;

				currentFileOffset += nodeData[level][i]->shape.GetSizeInBytes();
				currentEntryIndex++;
			}
		}

		// write materials
		{
			for (const auto& data : materialData)
			{
				const auto& materialName = data.first;
				const auto& mat = data.second;

				std::filesystem::path materialAbsolutePath = dataAbsoluteDirectory / mat.materialRelativePath;

				rapidjson::Document json;
				json.SetObject();

				rapidjson::Document::AllocatorType& alloc = json.GetAllocator();

				std::string materialTypeStr;
				if (mat.TransparencyFactor != 0)
				{
					materialTypeStr = "standard_transparent";
				}
				else
				{
					if (mat.TransparentColor.empty())
					{
						materialTypeStr = "standard";
					}
					else
					{
						materialTypeStr = "standard_cutoff";
					}
				}

				json.AddMember("asset_type", rapidjson::Value("material", alloc), alloc);
				json.AddMember("name", rapidjson::Value(materialName.c_str(), alloc), alloc);
				json.AddMember("material_type", rapidjson::Value(materialTypeStr.c_str(), alloc), alloc);

				{
					rapidjson::Value jsonBindingsEntry(rapidjson::kObjectType);
					jsonBindingsEntry.AddMember("DiffuseMap", rapidjson::Value(mat.DiffuseMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("EmissiveMap", rapidjson::Value(mat.EmissiveMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("EmissiveFactor", rapidjson::Value(mat.EmissiveFactor), alloc);
					jsonBindingsEntry.AddMember("AmbientMap", rapidjson::Value(mat.AmbientMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("AmbientFactor", rapidjson::Value(mat.AmbientFactor), alloc);
					jsonBindingsEntry.AddMember("NormalMap", rapidjson::Value(mat.NormalMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("TransparentColor", rapidjson::Value(mat.TransparentColor.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("TransparencyFactor", rapidjson::Value(mat.TransparencyFactor), alloc);
					jsonBindingsEntry.AddMember("SpecularMap", rapidjson::Value(mat.SpecularMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("SpecularFactor", rapidjson::Value(mat.SpecularFactor), alloc);
					jsonBindingsEntry.AddMember("ReflectionMap", rapidjson::Value(mat.ReflectionMap.c_str(), alloc), alloc);
					jsonBindingsEntry.AddMember("ReflectionFactor", rapidjson::Value(mat.ReflectionFactor), alloc);
					jsonBindingsEntry.AddMember("ShininessMap", rapidjson::Value(mat.ShininessMap.c_str(), alloc), alloc);

					jsonBindingsEntry.AddMember("TextureSampler", "linearWrap", alloc);

					json.AddMember("bindings", jsonBindingsEntry, alloc);
				}

				rapidjson::StringBuffer buffer;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
				json.Accept(writer);
				std::ofstream materialStream(materialAbsolutePath, std::ofstream::trunc);

				materialStream.write(buffer.GetString(), buffer.GetSize());

				materialStream.close();
			}
		}

		// write gameobject
		{
			rapidjson::Document json;
			json.SetObject();
			rapidjson::Document::AllocatorType& alloc = json.GetAllocator();

			CreateGameObjectJson(nodeData[0][0].get(), modelRelativePath, materialData, json, alloc);

			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
			writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
			json.Accept(writer);
			std::ofstream prefabStream(convertedModelPrefabPath, std::ofstream::trunc);

			prefabStream.write(buffer.GetString(), buffer.GetSize());

			prefabStream.close();
		}


		//write meshes
		{
			std::ofstream modelFileStream(convertedModelPath, std::ofstream::binary | std::ofstream::trunc);

			modelFileStream.seekp(0);
			modelFileStream.write(reinterpret_cast<const char*>(treeEntries.data()), treeEntries.size() * sizeof(TreeEntry));

			for (int level = 0; level < MAX_HIERARCHY_DEPTH; level++)
			{
				for (int i = 0; i < nodeData[level].size(); i++)
				{
					if (nodeData[level][i]->shape.GetSizeInBytes() == 0)
					{
						continue;
					}

					modelFileStream.seekp(nodeData[level][i]->dataFileOffset);
					const ShapeData& shape = nodeData[level][i]->shape;

					modelFileStream.write(reinterpret_cast<const char*>(&shape.m_header), sizeof(JoyEngine::MeshAssetHeader));
					modelFileStream.write(reinterpret_cast<const char*>(shape.m_vertices.data()), shape.m_header.vertexDataSize);
					modelFileStream.write(reinterpret_cast<const char*>(shape.m_indices.data()), shape.m_header.indexDataSize);
				}
			}

			if (modelFileStream.is_open())
			{
				modelFileStream.close();
			}
		}

		return true;
	}
}
