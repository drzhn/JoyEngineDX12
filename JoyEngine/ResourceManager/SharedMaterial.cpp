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

namespace JoyEngine
{
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

	void AbstractPipelineObject::CreateRootSignature(const std::vector<CD3DX12_ROOT_PARAMETER1>& rootParams)
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
			static_cast<uint32_t>(rootParams.size()),
			rootParams.empty() ? nullptr : rootParams.data(),
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

	ComputePipeline::ComputePipeline(GUID guid, ComputePipelineArgs args) :
		Resource(guid)
	{
		Shader* shaderPtr = JoyContext::Resource->LoadResource<Shader>(args.computeShaderGuid, JoyShaderTypeCompute);
		m_shader = shaderPtr;
		CreateRootSignature(args.rootParams);
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

	SharedMaterial::SharedMaterial(GUID guid) :
		Resource(guid) // UNUSED
	{
		rapidjson::Document json = JoyContext::Data->GetSerializedData(m_guid, sharedMaterial);

		m_shader = GUID::StringToGuid(json["shader"].GetString());

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
			ASSERT(false)
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
			ASSERT(false)
		}

		std::string blendStr = json["blend"].GetString();
		CD3DX12_BLEND_DESC blendDesc;
		switch (strHash(blendStr.c_str()))
		{
		case strHash("default"):
			blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			break;
		default:
			ASSERT(false)
		}

		std::string topologyStr = json["topology"].GetString();
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
		switch (strHash(topologyStr.c_str()))
		{
		case strHash("triangle"):
			topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
		default:
			ASSERT(false)
		}

		std::vector<DXGI_FORMAT> renderTargetsFormats(json["rtvFormats"].GetArray().Size());
		uint32_t i = 0;
		for (auto& format : json["rtvFormats"].GetArray())
		{
			std::string formatStr = format.GetString();
			switch (strHash(formatStr.c_str()))
			{
			case strHash("rgba8_unorm"):
				renderTargetsFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			default:
				ASSERT(false)
			}
			i++;
		}


		std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;
		std::vector<CD3DX12_ROOT_PARAMETER1> params;
		for (const auto& pair : m_shader->GetInputMap())
		{
			const std::string& name = pair.first;
			const ShaderInput& input = pair.second;

			if (name == "mvp")
			{
				params.emplace_back();
				params[params.size() - 1].InitAsConstants(
					sizeof(MVP) / 4, input.BindPoint, input.Space, input.Visibility);
				m_engineBindings.insert({static_cast<uint32_t>(params.size() - 1), ModelViewProjection});
			}
			else
			{
				D3D12_DESCRIPTOR_RANGE_TYPE type;
				switch (input.Type)
				{
				case D3D_SIT_CBUFFER: type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;
				case D3D_SIT_TEXTURE: type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;
				case D3D_SIT_SAMPLER: type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
					break;

				case D3D_SIT_UAV_RWTYPED:
				case D3D_SIT_STRUCTURED:
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
				ranges.emplace_back();
				ranges.back().Init(type, input.BindCount, input.BindPoint, input.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params.emplace_back();
				params[params.size() - 1].InitAsDescriptorTable(input.BindCount, &ranges.back(), input.Visibility);
			}
		}

		DXGI_FORMAT depthFormat = DXGI_FORMAT_D32_FLOAT;

		CreateRootSignature(params);
		CreateGraphicsPipeline(renderTargetsFormats, blendDesc, depthFormat, topology);
		JoyContext::Render->RegisterSharedMaterial(this);
	}

	SharedMaterial::SharedMaterial(const GUID guid, const SharedMaterialArgs args) :
		Resource(guid),
		m_hasVertexInput(args.hasVertexInput),
		m_depthTest(args.depthTest),
		m_depthWrite(args.depthWrite),
		m_depthComparisonFunc(args.depthComparisonFunc),
		m_cullMode(args.cullMode),
		m_engineBindings(args.engineBindings)
	{
		Shader* shaderPtr = JoyContext::Resource->LoadResource<Shader>(args.shader, args.shaderTypes);
		m_shader = shaderPtr;

		CreateRootSignature(args.rootParams);
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
