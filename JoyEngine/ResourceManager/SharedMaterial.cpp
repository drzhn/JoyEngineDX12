#include "SharedMaterial.h"

#include <rapidjson/document.h>

#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/RenderManager.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "Common/HashDefs.h"
#include "Utils/Log.h"

namespace JoyEngine
{
	SharedMaterial::SharedMaterial(GUID guid) :
		Resource(guid)
	{
		GraphicsPipelineArgs args = {};

		rapidjson::Document json = DataManager::Get()->GetSerializedData(m_guid, sharedMaterial);

		args.hasVertexInput = json["hasVertexInput"].GetBool();
		args.depthTest = json["depthTest"].GetBool();
		args.depthWrite = json["depthWrite"].GetBool();

		std::string depthCompStr = json["comparison"].GetString();
		switch (strHash(depthCompStr.c_str()))
		{
		case strHash("less_equal"):
			args.depthComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		default:
			ASSERT(false);
		}

		std::string cullModeStr = json["cull"].GetString();
		switch (strHash(cullModeStr.c_str()))
		{
		case strHash("back"):
			args.cullMode = D3D12_CULL_MODE_BACK;
			break;
		case strHash("front"):
			args.cullMode = D3D12_CULL_MODE_FRONT;
			break;
		case strHash("none"):
			args.cullMode = D3D12_CULL_MODE_NONE;
			break;
		default:
			ASSERT(false);
		}

		std::string blendStr = json["blend"].GetString();
		CD3DX12_BLEND_DESC blendDesc;
		switch (strHash(blendStr.c_str()))
		{
		case strHash("default"):
			blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			break;
		default:
			ASSERT(false);
		}

		std::string topologyStr = json["topology"].GetString();
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		switch (strHash(topologyStr.c_str()))
		{
		case strHash("triangle"):
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
			switch (strHash(typeStr.c_str()))
			{
			case strHash("vertex"): args.shaderTypes |= JoyShaderTypeVertex;
				break;
			case strHash("hull"): args.shaderTypes |= JoyShaderTypeHull;
				break;
			case strHash("domain"): args.shaderTypes |= JoyShaderTypeDomain;
				break;
			case strHash("geometry"): args.shaderTypes |= JoyShaderTypeGeometry;
				break;
			case strHash("pixel"): args.shaderTypes |= JoyShaderTypePixel;
				break;
			case strHash("amplification"): args.shaderTypes |= JoyShaderTypeAmplification;
				break;
			case strHash("mesh"): args.shaderTypes |= JoyShaderTypeMesh;
				break;
			default:
				ASSERT(false);
			}
		}

		args.shader = GUID::StringToGuid(json["shader"].GetString());

		args.depthFormat = RenderManager::Get()->GetDepthFormat();
		args.blendDesc = blendDesc;
		args.topology = topology;
		args.renderTargetsFormats[0] = RenderManager::Get()->GetMainColorFormat();
		args.renderTargetsFormatsSize = 1;

		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(args);

		RenderManager::Get()->RegisterSharedMaterial(this);
	}

	SharedMaterial::SharedMaterial(GUID guid, GraphicsPipelineArgs args) :
		Resource(guid)
	{
		GUID graphicsPipelineGuid = GUID::Random();
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
