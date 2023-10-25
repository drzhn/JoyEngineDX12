#include "SharedMaterial.h"

#include <rapidjson/document.h>

#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/RenderManager.h"


#include <d3d12.h>

#include "Common/HashDefs.h"
#include "Utils/Log.h"

namespace JoyEngine
{
	SharedMaterial::SharedMaterial(const char* path) :
		Resource(path)
	{
		GraphicsPipelineArgs args = {};

		rapidjson::Document json = DataManager::Get()->GetSerializedData(path, AssetType::SharedMaterial);

		args.hasVertexInput = json["hasVertexInput"].GetBool();
		args.depthTest = json["depthTest"].GetBool();
		args.depthWrite = json["depthWrite"].GetBool();

		std::string depthCompStr = json["comparison"].GetString();
		switch (StrHash32(depthCompStr.c_str()))
		{
		case StrHash32("less_equal"):
			args.depthComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		default:
			ASSERT(false);
		}

		std::string cullModeStr = json["cull"].GetString();
		switch (StrHash32(cullModeStr.c_str()))
		{
		case StrHash32("back"):
			args.cullMode = D3D12_CULL_MODE_BACK;
			break;
		case StrHash32("front"):
			args.cullMode = D3D12_CULL_MODE_FRONT;
			break;
		case StrHash32("none"):
			args.cullMode = D3D12_CULL_MODE_NONE;
			break;
		default:
			ASSERT(false);
		}

		std::string blendStr = json["blend"].GetString();
		CD3DX12_BLEND_DESC blendDesc;
		switch (StrHash32(blendStr.c_str()))
		{
		case StrHash32("default"):
			blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			break;
		default:
			ASSERT(false);
		}

		std::string topologyStr = json["topology"].GetString();
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		switch (StrHash32(topologyStr.c_str()))
		{
		case StrHash32("triangle"):
			topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		default:
			ASSERT(false);
		}

		// Shader creation
		args.shaderTypes = 0;

		for (auto& type : json["shaderTypes"].GetArray())
		{
			std::string typeStr = type.GetString();
			switch (StrHash32(typeStr.c_str()))
			{
			case StrHash32("vertex"): args.shaderTypes |= JoyShaderTypeVertex;
				break;
			case StrHash32("hull"): args.shaderTypes |= JoyShaderTypeHull;
				break;
			case StrHash32("domain"): args.shaderTypes |= JoyShaderTypeDomain;
				break;
			case StrHash32("geometry"): args.shaderTypes |= JoyShaderTypeGeometry;
				break;
			case StrHash32("pixel"): args.shaderTypes |= JoyShaderTypePixel;
				break;
			case StrHash32("amplification"): args.shaderTypes |= JoyShaderTypeAmplification;
				break;
			case StrHash32("mesh"): args.shaderTypes |= JoyShaderTypeMesh;
				break;
			default:
				ASSERT(false);
			}
		}

		args.shaderPath = json["shader"].GetString();

		args.depthFormat = RenderManager::Get()->GetDepthFormat();
		args.blendDesc = blendDesc;
		args.topology = topology;
		args.renderTargetsFormats[0] = RenderManager::Get()->GetMainColorFormat();
		args.renderTargetsFormatsSize = 1;

		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(args);

		RenderManager::Get()->RegisterSharedMaterial(this);
	}

	SharedMaterial::SharedMaterial(uint64_t id, GraphicsPipelineArgs args) :
		Resource(id)
	{
		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(args);

		RenderManager::Get()->RegisterSharedMaterial(this);
	}

	SharedMaterial::~SharedMaterial()
	{
		RenderManager::Get()->UnregisterSharedMaterial(this);
	}


	bool SharedMaterial::IsLoaded() const noexcept
	{
		return true;
	}

	void SharedMaterial::RegisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		m_meshRenderers.insert(meshRenderer);
	}

	void SharedMaterial::UnregisterMeshRenderer(MeshRenderer* meshRenderer)
	{
		if (m_meshRenderers.find(meshRenderer) == m_meshRenderers.end())
		{
			ASSERT(false);
		}
		m_meshRenderers.erase(meshRenderer);
	}

	std::set<MeshRenderer*>& SharedMaterial::GetMeshRenderers()
	{
		return m_meshRenderers;
	}

	GraphicsPipeline* SharedMaterial::GetGraphicsPipeline() const
	{
		return m_graphicsPipeline.get();
	}

	uint32_t SharedMaterial::GetBindingIndexByHash(uint32_t hash) const
	{
		return m_graphicsPipeline->GetBindingIndexByHash(hash);
	}
}
