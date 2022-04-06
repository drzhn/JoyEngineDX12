#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>

#include "JoyContext.h"
#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/JoyTypes.h"
#include "RenderManager/RenderManager.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "Common/HashDefs.h"

using Microsoft::WRL::ComPtr;

#define DESCRIPTOR_ARRAY_SIZE 32

namespace JoyEngine
{
	// =============================== ABSTRACT PIPELINE =================================

	const ShaderInput& AbstractPipelineObject::GetShaderInputByName(const std::string& name) const
	{
		ASSERT(m_shader->GetInputMap().find(name) != m_shader->GetInputMap().end());
		return m_shader->GetInputMap().find(name)->second;
	}

	uint32_t AbstractPipelineObject::GetRootIndexByName(const std::string& name) const
	{
		ASSERT(m_rootIndices.find(name) != m_rootIndices.end());
		return m_rootIndices.find(name)->second;
	}

	void AbstractPipelineObject::CreateShaderAndRootSignature(GUID shaderGuid, ShaderTypeFlags shaderTypes)
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCRIPTOR_ARRAY_SIZE];
		uint32_t rangesIndex = 0;

		CD3DX12_ROOT_PARAMETER1 params[DESCRIPTOR_ARRAY_SIZE];
		uint32_t paramsIndex = 0;
		{
			Shader* shaderPtr = JoyContext::Resource->LoadResource<Shader>(
				shaderGuid,
				shaderTypes
			);
			m_shader = shaderPtr;

			for (const auto& pair : m_shader->GetInputMap())
			{
				const std::string& name = pair.first;
				const ShaderInput& input = pair.second;

				if (name == "mvp")
				{
					params[paramsIndex].InitAsConstants(
						sizeof(MVP) / 4, input.BindPoint, input.Space, input.Visibility);
					m_engineBindings.insert({static_cast<uint32_t>(paramsIndex), ModelViewProjection});
					paramsIndex++;
				}
				else
				{
					D3D12_DESCRIPTOR_RANGE_TYPE type;
					switch (input.Type)
					{
					case D3D_SIT_CBUFFER: type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
						break;

					case D3D_SIT_STRUCTURED: // i dunno, don't ask me
					case D3D_SIT_TEXTURE: type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
						break;
					case D3D_SIT_SAMPLER: type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
						break;

					case D3D_SIT_UAV_RWTYPED:
					case D3D_SIT_UAV_RWSTRUCTURED:
					case D3D_SIT_UAV_RWBYTEADDRESS:
					case D3D_SIT_UAV_APPEND_STRUCTURED:
					case D3D_SIT_UAV_CONSUME_STRUCTURED:
					case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
					case D3D_SIT_UAV_FEEDBACKTEXTURE:
						type = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
						break;

					case D3D_SIT_TBUFFER:
					case D3D_SIT_BYTEADDRESS:
					case D3D_SIT_RTACCELERATIONSTRUCTURE:
					default:
						ASSERT(false);
					}
					ranges[rangesIndex].Init(type, input.BindCount, input.BindPoint, input.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
					params[paramsIndex].InitAsDescriptorTable(input.BindCount, &ranges[rangesIndex], input.Visibility);
					m_rootIndices.insert({name, paramsIndex});
					rangesIndex++;
					paramsIndex++;
				}
			}
		}

		CreateRootSignature(params, paramsIndex);
	}

	void AbstractPipelineObject::CreateRootSignature(CD3DX12_ROOT_PARAMETER1* params, uint32_t paramsCount)
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(JoyContext::Graphics->GetDevice()->CheckFeatureSupport(
			D3D12_FEATURE_ROOT_SIGNATURE,
			&featureData,
			sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}


		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			paramsCount,
			params,
			0,
			nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		HRESULT result = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error);
		if (FAILED(result) && error != nullptr)
		{
			const char* errorMsg = static_cast<const char*>(error->GetBufferPointer());
			OutputDebugStringA(errorMsg);
			ASSERT(false);
		}
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}

	// =============================== COMPUTE PIPELINE =================================

	ComputePipeline::ComputePipeline(GUID guid, ComputePipelineArgs args) :
		Resource(guid)
	{
		CreateShaderAndRootSignature(args.computeShaderGuid, JoyShaderTypeCompute);
		CreateComputePipeline();
	}

	void ComputePipeline::CreateComputePipeline()
	{
		const D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {
			m_rootSignature.Get(),
			CD3DX12_SHADER_BYTECODE(m_shader->GetComputeShadeModule().Get()),
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};
		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// =============================== SHARED MATERIAL =================================


	std::vector<D3D12_INPUT_ELEMENT_DESC> SharedMaterial::m_inputLayout = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	SharedMaterial::SharedMaterial(GUID guid) :
		Resource(guid)
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

		m_hasVertexInput = json["hasVertexInput"].GetBool();
		m_depthTest = json["depthTest"].GetBool();
		m_depthWrite = json["depthWrite"].GetBool();

		std::string depthCompStr = json["comparison"].GetString();
		switch (strHash(depthCompStr.c_str()))
		{
		case strHash("less_equal"):
			m_depthComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		default:
			ASSERT(false);
		}

		std::string cullModeStr = json["cull"].GetString();
		switch (strHash(cullModeStr.c_str()))
		{
		case strHash("back"):
			m_cullMode = D3D12_CULL_MODE_BACK;
			break;
		case strHash("front"):
			m_cullMode = D3D12_CULL_MODE_FRONT;
			break;
		case strHash("none"):
			m_cullMode = D3D12_CULL_MODE_NONE;
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

		std::vector<DXGI_FORMAT> renderTargetsFormats
		{
			JoyEngine::RenderManager::GetMainColorFormat() // materials created form GUIDs can only write to one main color rtv
		};

		// Shader creation
		ShaderTypeFlags shaderTypeFlags = 0;
		for (auto& type : json["shaderTypes"].GetArray())
		{
			std::string typeStr = type.GetString();
			switch (strHash(typeStr.c_str()))
			{
			case strHash("vertex"): shaderTypeFlags |= JoyShaderTypeVertex;
				break;
			case strHash("hull"): shaderTypeFlags |= JoyShaderTypeHull;
				break;
			case strHash("domain"): shaderTypeFlags |= JoyShaderTypeDomain;
				break;
			case strHash("geometry"): shaderTypeFlags |= JoyShaderTypeGeometry;
				break;
			case strHash("pixel"): shaderTypeFlags |= JoyShaderTypePixel;
				break;
			case strHash("amplification"): shaderTypeFlags |= JoyShaderTypeAmplification;
				break;
			case strHash("mesh"): shaderTypeFlags |= JoyShaderTypeMesh;
				break;
			case strHash("compute"): shaderTypeFlags |= JoyShaderTypeCompute;
				break;
			default:
				ASSERT(false);
			}
		}

		GUID shaderGuid = GUID::StringToGuid(json["shader"].GetString());
		CreateShaderAndRootSignature(shaderGuid, shaderTypeFlags);

		DXGI_FORMAT depthFormat = RenderManager::GetDepthFormat();
		CreateGraphicsPipeline(renderTargetsFormats, blendDesc, depthFormat, topology);

		JoyContext::Render->RegisterSharedMaterial(this);
	}

	SharedMaterial::SharedMaterial(const GUID guid, const SharedMaterialArgs args) :
		Resource(guid),
		m_hasVertexInput(args.hasVertexInput),
		m_depthTest(args.depthTest),
		m_depthWrite(args.depthWrite),
		m_depthComparisonFunc(args.depthComparisonFunc),
		m_cullMode(args.cullMode)
	{
		CreateShaderAndRootSignature(args.shader, args.shaderTypes);
		CreateGraphicsPipeline(args.renderTargetsFormats, args.blendDesc, args.depthFormat, args.topology);

		JoyContext::Render->RegisterSharedMaterial(this);
	}

	void SharedMaterial::CreateGraphicsPipeline(
		const std::vector<DXGI_FORMAT>& renderTargetsFormats,
		CD3DX12_BLEND_DESC blendDesc,
		DXGI_FORMAT depthFormat,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
	{
		// Create the vertex input layout

		CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterizerDesc.CullMode = m_cullMode;

		constexpr CD3DX12_SHADER_BYTECODE emptyBytecode = {};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {
			m_rootSignature.Get(),
			m_shader->GetShaderType() & JoyShaderTypeVertex ? CD3DX12_SHADER_BYTECODE(m_shader->GetVertexShadeModule().Get()) : emptyBytecode,
			m_shader->GetShaderType() & JoyShaderTypeVertex ? CD3DX12_SHADER_BYTECODE(m_shader->GetFragmentShadeModule().Get()) : emptyBytecode,
			{},
			{},
			m_shader->GetShaderType() & JoyShaderTypeGeometry ? CD3DX12_SHADER_BYTECODE(m_shader->GetGeometryShadeModule().Get()) : emptyBytecode,
			{},
			blendDesc,
			UINT_MAX,
			rasterizerDesc,
			{
				m_depthTest ? TRUE : FALSE,
				m_depthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
				m_depthComparisonFunc,
				FALSE,
				0,
				0,
				D3D12_DEPTH_STENCILOP_DESC({}),
				D3D12_DEPTH_STENCILOP_DESC({})
			},
			{
				m_hasVertexInput ? m_inputLayout.data() : nullptr,
				m_hasVertexInput ? static_cast<uint32_t>(m_inputLayout.size()) : 0
			},
			{},
			topology,
			// i'm sorry
			static_cast<uint32_t>(renderTargetsFormats.size()),
			{
				renderTargetsFormats.size() > 0 ? renderTargetsFormats[0] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 1 ? renderTargetsFormats[1] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 2 ? renderTargetsFormats[2] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 3 ? renderTargetsFormats[3] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 4 ? renderTargetsFormats[4] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 5 ? renderTargetsFormats[5] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 6 ? renderTargetsFormats[6] : DXGI_FORMAT_UNKNOWN,
				renderTargetsFormats.size() > 7 ? renderTargetsFormats[7] : DXGI_FORMAT_UNKNOWN
			},
			depthFormat,
			{1, 0},
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&m_pipelineState)));
	}


	SharedMaterial::~SharedMaterial()
	{
		JoyContext::Render->UnregisterSharedMaterial(this);
	}


	bool SharedMaterial::IsLoaded() const noexcept
	{
		return m_shader->IsLoaded();
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

	std::map<uint32_t, EngineBindingType>& SharedMaterial::GetEngineBindings()
	{
		return m_engineBindings;
	}
}
