#include "SharedMaterial.h"

#include <array>

#include <rapidjson/document.h>


#include "Common/SerializationUtils.h"
#include "DataManager/DataManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "CommonEngineStructs.h"
#include "RenderManager/RenderManager.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "Common/HashDefs.h"
#include "DescriptorManager/DescriptorManager.h"
#include "Utils/Log.h"

using Microsoft::WRL::ComPtr;

#define DESCRIPTOR_ARRAY_SIZE 32
#define MAX_SHADER_LIB_EXPORTS 8
#define MAX_STATE_SUBOBJECTS 16

namespace JoyEngine
{
	const wchar_t* g_hitGroupName = L"HitGroup";

	RaytracingPipeline::RaytracingPipeline(const RaytracingPipelineArgs& args)
	{
		m_raytracingShader = ResourceManager::Get()->LoadResource<Shader>(
			args.raytracingShaderGuid,
			JoyShaderTypeRaytracing
		);

		D3D12_STATE_SUBOBJECT stateSubobjects[MAX_STATE_SUBOBJECTS];
		uint32_t subobjectIndex = 0;

		// global root signature subobject;
		m_globalInputContainer.InitContainer(m_raytracingShader.Get()->GetInputMap(), D3D12_ROOT_SIGNATURE_FLAG_NONE);

		D3D12_GLOBAL_ROOT_SIGNATURE globalRootSignatureDesc{
			.pGlobalRootSignature = m_globalInputContainer.GetRootSignature().Get()
		};

		auto& globalRootSignatureSubobject = stateSubobjects[subobjectIndex];
		globalRootSignatureSubobject = {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
			.pDesc = &globalRootSignatureDesc
		};

		subobjectIndex++;

		// dxil export and association descs
		D3D12_EXPORT_DESC exportDescs[MAX_SHADER_LIB_EXPORTS];
		D3D12_LOCAL_ROOT_SIGNATURE localRootSignatureDescs[MAX_SHADER_LIB_EXPORTS];
		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION subobjectToExportsAssociationDescs[MAX_SHADER_LIB_EXPORTS];
		LPCWSTR exports[MAX_SHADER_LIB_EXPORTS];
		int exportIndex = 0;
		for (const auto& inputMap : m_raytracingShader.Get()->GetLocalInputMaps())
		{
			// export desc
			{
				exportDescs[exportIndex] = D3D12_EXPORT_DESC
				{
					.Name = inputMap.first.c_str(),
					.ExportToRename = nullptr,
					.Flags = D3D12_EXPORT_FLAG_NONE
				};
				m_localInputContainers.insert({
					inputMap.first,
					ShaderInputContainer(inputMap.second, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE)
				});
			}

			// local root signature subobject
			{
				localRootSignatureDescs[exportIndex] = {
					.pLocalRootSignature = m_localInputContainers[inputMap.first].GetRootSignature().Get()
				};

				auto& localRootSignatureSubobject = stateSubobjects[subobjectIndex];
				localRootSignatureSubobject = {
					.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE,
					.pDesc = &localRootSignatureDescs[exportIndex]
				};

				subobjectIndex++;
			}

			// local root signature association
			{
				exports[exportIndex] = inputMap.first.c_str();
				subobjectToExportsAssociationDescs[exportIndex] = {
					.pSubobjectToAssociate = &stateSubobjects[subobjectIndex - 1],
					.NumExports = 1,
					.pExports = &exports[exportIndex]
				};

				auto& signatureAssociationSubobject = stateSubobjects[subobjectIndex];
				signatureAssociationSubobject = {
					.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
					.pDesc = &subobjectToExportsAssociationDescs[exportIndex]
				};
				subobjectIndex++;
			}

			exportIndex++;
		}

		// dxil lib
		D3D12_DXIL_LIBRARY_DESC libraryDesc{
			.DXILLibrary = CD3DX12_SHADER_BYTECODE(m_raytracingShader->GetRaytracingShadeModule().Get()),
			.NumExports = static_cast<uint32_t>(m_raytracingShader.Get()->GetLocalInputMaps().size()),
			.pExports = exportDescs
		};

		auto& dxilLibrarySubobject = stateSubobjects[subobjectIndex];
		dxilLibrarySubobject = {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY,
			.pDesc = &libraryDesc
		};
		subobjectIndex++;

		// raystracing shader config
		D3D12_RAYTRACING_SHADER_CONFIG raytracingShaderConfig{
			.MaxPayloadSizeInBytes = 4 * sizeof(float), // float4 color
			.MaxAttributeSizeInBytes = 2 * sizeof(float) // float2 barycentrics
		};

		auto& raytracingShaderConfigSubobject = stateSubobjects[subobjectIndex];
		raytracingShaderConfigSubobject = {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG,
			.pDesc = &raytracingShaderConfig
		};

		subobjectIndex++;

		// raytracing Pipeline Config Subobject
		D3D12_RAYTRACING_PIPELINE_CONFIG raytracingPipelineConfig{
			.MaxTraceRecursionDepth = 1 // ~ primary rays only. 
		};

		auto& raytracingPipelineConfigSubobject = stateSubobjects[subobjectIndex];
		raytracingPipelineConfigSubobject = {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG,
			.pDesc = &raytracingPipelineConfig
		};

		subobjectIndex++;

		// hit group subobject
		D3D12_HIT_GROUP_DESC hitGroupDesc = {
			.HitGroupExport = g_hitGroupName,
			.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
			.AnyHitShaderImport = m_raytracingShader->GetFunctionNameByType(D3D12_SHVER_ANY_HIT_SHADER),
			.ClosestHitShaderImport = m_raytracingShader->GetFunctionNameByType(D3D12_SHVER_CLOSEST_HIT_SHADER),
			.IntersectionShaderImport = m_raytracingShader->GetFunctionNameByType(D3D12_SHVER_INTERSECTION_SHADER)
		};

		auto& hitGroupSubobject = stateSubobjects[subobjectIndex];
		hitGroupSubobject = {
			.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP,
			.pDesc = &hitGroupDesc
		};

		subobjectIndex++;


		// ===================================

		D3D12_STATE_OBJECT_DESC stateObjectDesc{
			.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
			.NumSubobjects = subobjectIndex,
			.pSubobjects = stateSubobjects
		};

		ASSERT_SUCC(GraphicsManager::Get()->GetDevice()->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&m_stateObject)));
	}

	// ================ SHADER INPUT CONTAINER ===========================

	uint32_t ShaderInputContainer::GetBindingIndexByName(const std::string& name) const
	{
		ASSERT(m_rootIndices.contains(strHash(name.c_str())));
		return m_rootIndices.find(strHash(name.c_str()))->second;
	}

	uint32_t ShaderInputContainer::GetBindingIndexByHash(const uint32_t hash) const
	{
		if (!m_rootIndices.contains(hash))
		{
			Logger::LogFormat("Warning: pipeline doesn't contain hash %d", hash);
			return -1;
		}
		return m_rootIndices.find(hash)->second;
	}

	const std::map<uint32_t, EngineBindingType>& ShaderInputContainer::GetEngineBindings() const
	{
		return m_engineBindings;
	}

	void ShaderInputContainer::InitContainer(
		const ShaderInputMap& inputMap,
		D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCRIPTOR_ARRAY_SIZE];
		uint32_t rangesIndex = 0;

		CD3DX12_ROOT_PARAMETER1 params[DESCRIPTOR_ARRAY_SIZE];
		uint32_t paramsIndex = 0;

		for (const auto& pair : inputMap)
		{
			const std::string& name = pair.first;
			const ShaderInput& input = pair.second;

			if (name == "objectIndex")
			{
				params[paramsIndex].InitAsConstants(
					sizeof(uint32_t) / 4, input.BindPoint, input.Space, input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::ObjectIndexData});
				paramsIndex++;
			}
			else if (name == "viewProjectionData")
			{
				params[paramsIndex].InitAsConstants(
					sizeof(ViewProjectionMatrixData) / 4, input.BindPoint, input.Space, input.Visibility);

				m_engineBindings.insert({paramsIndex, EngineBindingType::ViewProjectionMatrixData});
				paramsIndex++;
			}
			else if (name == "objectMatricesData")
			{
				ranges[rangesIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, input.BindPoint, input.Space,
				                         D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::ModelMatrixData});
				rangesIndex++;
				paramsIndex++;
			}
			else if (name == "engineData")
			{
				ranges[rangesIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, input.BindPoint, input.Space,
				                         D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_engineBindings.insert({paramsIndex, EngineBindingType::EngineData});
				rangesIndex++;
				paramsIndex++;
			}
			else
			{
				D3D12_DESCRIPTOR_RANGE_TYPE type;
				switch (input.Type)
				{
				case D3D_SIT_CBUFFER: 
					type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
					break;

				case D3D_SIT_STRUCTURED:
				case D3D_SIT_TEXTURE:
				case D3D_SIT_RTACCELERATIONSTRUCTURE:
					type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
					break;
				case D3D_SIT_SAMPLER: 
					type = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
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
				default:
					ASSERT(false);
				}
				ranges[rangesIndex].Init(type, input.BindCount == 0 ? READONLY_TEXTURES_COUNT : input.BindCount,
				                         input.BindPoint, input.Space, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
				params[paramsIndex].InitAsDescriptorTable(1, &ranges[rangesIndex], input.Visibility);
				m_rootIndices.insert({strHash(name.c_str()), paramsIndex});
				rangesIndex++;
				paramsIndex++;
			}
		}

		// TODO should I make compatibility with 1.0?
		ASSERT(GraphicsManager::Get()->GetHighestRootSignatureVersion() == D3D_ROOT_SIGNATURE_VERSION_1_1);
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			paramsIndex,
			params,
			0,
			nullptr,
			flags);

		ComPtr<ID3DBlob> signature;
		ID3D10Blob** errorPtr = nullptr;
#ifdef _DEBUG
		ComPtr<ID3DBlob> error;
		errorPtr = &error;
#endif

		const HRESULT result = D3DX12SerializeVersionedRootSignature(
			&rootSignatureDesc,
			GraphicsManager::Get()->GetHighestRootSignatureVersion(),
			&signature,
			errorPtr);

#ifdef _DEBUG
		ASSERT_DESC(result == S_OK, static_cast<const char*>(error->GetBufferPointer()));
#endif

		ASSERT_SUCC(GraphicsManager::Get()->GetDevice()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)));
	}

	// =============================== ABSTRACT PIPELINE =================================

	AbstractPipelineObject::AbstractPipelineObject(
		GUID shaderGuid,
		ShaderTypeFlags shaderTypes,
		D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		m_shader = ResourceManager::Get()->LoadResource<Shader>(shaderGuid, shaderTypes);
		m_inputContainer.InitContainer(m_shader.Get()->GetInputMap(), flags);
	}

	ShaderInput const* AbstractPipelineObject::GetShaderInputByName(const std::string& name) const
	{
		if (m_shader->GetInputMap().contains(name))
		{
			return &m_shader->GetInputMap().find(name)->second;
		}
		return nullptr;
	}

	// =============================== COMPUTE PIPELINE =================================

	ComputePipeline::ComputePipeline(const ComputePipelineArgs& args):
		AbstractPipelineObject(
			args.computeShaderGuid,
			JoyShaderTypeCompute,
			D3D12_ROOT_SIGNATURE_FLAG_NONE)
	{
		const D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc = {
			m_inputContainer.GetRootSignature().Get(),
			CD3DX12_SHADER_BYTECODE(m_shader->GetComputeShadeModule().Get()),
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};
		ASSERT_SUCC(
			GraphicsManager::Get()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&
				m_pipelineState)));
	}

	// =============================== SHARED MATERIAL =================================


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

	// =============================== GRAPHICS PIPELINE =================================

	std::vector<D3D12_INPUT_ELEMENT_DESC> GraphicsPipeline::m_inputLayout = {
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

	GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineArgs& args) :
		AbstractPipelineObject(
			args.shader,
			args.shaderTypes,
			args.hasVertexInput ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE),
		m_topology(args.topology),
		m_hasVertexInput(args.hasVertexInput),
		m_depthTest(args.depthTest),
		m_depthWrite(args.depthWrite),
		m_depthComparisonFunc(args.depthComparisonFunc),
		m_cullMode(args.cullMode)
	{
		// Create the vertex input layout

		CD3DX12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterizerDesc.CullMode = m_cullMode;

		constexpr CD3DX12_SHADER_BYTECODE emptyBytecode = {};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {
			m_inputContainer.GetRootSignature().Get(),
			m_shader->GetShaderType() & JoyShaderTypeVertex
				? CD3DX12_SHADER_BYTECODE(m_shader->GetVertexShadeModule().Get())
				: emptyBytecode,
			m_shader->GetShaderType() & JoyShaderTypeVertex
				? CD3DX12_SHADER_BYTECODE(m_shader->GetFragmentShadeModule().Get())
				: emptyBytecode,
			{},
			{},
			m_shader->GetShaderType() & JoyShaderTypeGeometry
				? CD3DX12_SHADER_BYTECODE(m_shader->GetGeometryShadeModule().Get())
				: emptyBytecode,
			{},
			args.blendDesc,
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
			args.topology,
			args.renderTargetsFormatsSize,
			{
				args.renderTargetsFormatsSize > 0 ? args.renderTargetsFormats[0] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 1 ? args.renderTargetsFormats[1] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 2 ? args.renderTargetsFormats[2] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 3 ? args.renderTargetsFormats[3] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 4 ? args.renderTargetsFormats[4] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 5 ? args.renderTargetsFormats[5] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 6 ? args.renderTargetsFormats[6] : DXGI_FORMAT_UNKNOWN,
				args.renderTargetsFormatsSize > 7 ? args.renderTargetsFormats[7] : DXGI_FORMAT_UNKNOWN
			},
			args.depthFormat,
			{1, 0},
			0,
			{},
			D3D12_PIPELINE_STATE_FLAG_NONE
		};

		ASSERT_SUCC(
			GraphicsManager::Get()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&
				m_pipelineState)));
	}
}
