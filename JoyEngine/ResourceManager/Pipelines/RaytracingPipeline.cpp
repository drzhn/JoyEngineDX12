#include "RaytracingPipeline.h"

#include "d3dx12.h"
#include "GraphicsManager/GraphicsManager.h"
#include "ResourceManager/ResourceManager.h"

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
			const LPCWSTR functionName = m_raytracingShader.Get()->GetFunctionNameByType(inputMap.first);

			// export desc
			{
				exportDescs[exportIndex] = D3D12_EXPORT_DESC
				{
					.Name = functionName,
					.ExportToRename = nullptr,
					.Flags = D3D12_EXPORT_FLAG_NONE
				};
				m_localInputContainers.insert({
					inputMap.first,
					ShaderInputContainer(inputMap.second.inputMap, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE)
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
				exports[exportIndex] = functionName;
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

		// ================ Shader tables ================

		ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
		ASSERT_SUCC(m_stateObject.As(&stateObjectProperties));
		void* rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(m_raytracingShader->GetFunctionNameByType(D3D12_SHVER_RAY_GENERATION_SHADER));
		void* missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(m_raytracingShader->GetFunctionNameByType(D3D12_SHVER_MISS_SHADER));
		void* hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(g_hitGroupName);

		uint32_t recordSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		for (const auto & pair : m_raytracingShader.Get()->GetLocalInputMaps().at(D3D12_SHVER_RAY_GENERATION_SHADER).inputMap)
		{
			const ShaderInput& input = pair.second;
			recordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE); // we don't use pushconstants yet in shader tables. 
		}
	}
}
