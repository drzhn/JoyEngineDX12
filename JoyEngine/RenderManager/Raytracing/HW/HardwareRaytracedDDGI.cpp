#include "HardwareRaytracedDDGI.h"

#include "CommonEngineStructs.h"
#include "Common/HashDefs.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/Raytracing/SW/SoftwareRaytracedDDGI.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	HardwareRaytracedDDGI::HardwareRaytracedDDGI(
		const RaytracedDDGIDataContainer& dataContainer,
		DXGI_FORMAT mainColorFormat,
		DXGI_FORMAT swapchainFormat,
		uint32_t width,
		uint32_t height):
		m_dataContainer(dataContainer),
#if defined(HW_CAMERA_TRACE)
		m_raytracedTextureWidth(width),
		m_raytracedTextureHeight(height)
#else
		m_raytracedTextureWidth(g_raytracedProbesData.gridX* g_raytracedProbesData.gridY* g_raytracedProbesData.gridZ),
		m_raytracedTextureHeight(DDGI_RAYS_COUNT)
#endif
	{
		m_testTexture = std::make_unique<UAVTexture>(
			m_raytracedTextureWidth,
			m_raytracedTextureHeight,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);

		float border = 0.1f;

		m_raytracingPipeline = std::make_unique<RaytracingPipeline>(RaytracingPipelineArgs{
			GUID::StringToGuid("b2597599-94ef-43ed-abd8-46d3adbb75d4")
		});
	}

	void HardwareRaytracedDDGI::UploadSceneData()
	{
		TIME_PERF("Uploading hardware DDGI scene data");

		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC*> raytracingGeometryDescs;

		raytracingGeometryDescs.reserve(1000); // TODO more pretty way to store this data. 
		for (auto const& sm : m_dataContainer.GetSceneSharedMaterials())
		{
			for (const auto& mr : sm->GetMeshRenderers())
			{
				if (!mr->IsStatic()) continue;
				raytracingGeometryDescs.push_back(mr->GetMesh()->GetRaytracingGeometryDescPtr());
			}
		}

		// Top Level Inputs
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device5-getraytracingaccelerationstructureprebuildinfo
			// "It may not inspect/dereference any GPU virtual addresses...
			// ...rather it can only depend on overall properties, such as the number of triangles, number of instances etc."
			.InstanceDescs = 0
		};

		GraphicsManager::Get()->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
		ASSERT(topLevelPrebuildInfo.ScratchDataSizeInBytes > 0);

		// Bottom Level Inputs
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
			.NumDescs = static_cast<uint32_t>(raytracingGeometryDescs.size()),
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY_OF_POINTERS,
			.ppGeometryDescs = raytracingGeometryDescs.data()
		};
		GraphicsManager::Get()->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		ASSERT(bottomLevelPrebuildInfo.ScratchDataSizeInBytes > 0);


		std::unique_ptr<UAVGpuBuffer> accelerationScratch = std::make_unique<UAVGpuBuffer>(
			1,
			std::max(
				topLevelPrebuildInfo.ScratchDataSizeInBytes,
				bottomLevelPrebuildInfo.ScratchDataSizeInBytes)
		);

		m_accelerationTop = std::make_unique<UAVGpuBuffer>(
			1,
			topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
		);

		m_accelerationBottom = std::make_unique<UAVGpuBuffer>(
			1,
			bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
		);

		// Create an instance desc for the bottom-level acceleration structure
		D3D12_RAYTRACING_INSTANCE_DESC raytracingInstanceDesc = {
			.Transform = {
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0
			},
			.InstanceID = 0,
			.InstanceMask = 1,
			.InstanceContributionToHitGroupIndex = 0,
			.Flags = 0,
			.AccelerationStructure = m_accelerationBottom->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress()
		};

		std::unique_ptr<Buffer> instanceDescsBuffer = std::make_unique<Buffer>(
			sizeof(raytracingInstanceDesc),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_FLAG_NONE
		);

		instanceDescsBuffer->SetCPUData(&raytracingInstanceDesc, 0, sizeof(raytracingInstanceDesc));

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
			.DestAccelerationStructureData = m_accelerationBottom->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress(),
			.Inputs = bottomLevelInputs,
			.SourceAccelerationStructureData = 0,
			.ScratchAccelerationStructureData = accelerationScratch->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress()
		};

		topLevelInputs.InstanceDescs = instanceDescsBuffer->GetBufferResource()->GetGPUVirtualAddress();
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {
			.DestAccelerationStructureData = m_accelerationTop->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress(),
			.Inputs = topLevelInputs,
			.SourceAccelerationStructureData = 0,
			.ScratchAccelerationStructureData = accelerationScratch->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress()
		};

		auto commandList = m_dataContainer.GetDispatcher()->GetCommandList();

		commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
		GraphicsUtils::UAVBarrier(commandList, m_accelerationBottom->GetBuffer()->GetBufferResource().Get());
		commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

		m_dataContainer.GetDispatcher()->ExecuteAndWait();
	}

	void HardwareRaytracedDDGI::ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const
	{
		commandList->SetComputeRootSignature(m_raytracingPipeline->GetGlobalInputContainer()->GetRootSignature().Get());

		commandList->SetComputeRootShaderResourceView(
			m_raytracingPipeline->GetGlobalInputContainer()->GetBindingIndexByHash(strHash("g_SceneAccelerationStructure")),
			m_accelerationTop->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress());

		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "g_OutputRenderTarget", m_testTexture->GetUAV());
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "g_engineData", EngineMaterialProvider::Get()->GetEngineDataView(frameIndex));
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "raytracedProbesData", m_dataContainer.GetProbesDataView(frameIndex));
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "textures", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());

		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "meshData", m_dataContainer.GetMeshDataView());
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "objectVertices", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "objectIndices", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "materials", EngineMaterialProvider::Get()->GetMaterialsDataView());

		const D3D12_DISPATCH_RAYS_DESC dispatchDesc = {
			.RayGenerationShaderRecord = {
				.StartAddress = m_raytracingPipeline->GetShaderTableByType(ShaderTableRaygen)->GetAddress(),
				.SizeInBytes = m_raytracingPipeline->GetShaderTableByType(ShaderTableRaygen)->GetSize(),
			},
			.MissShaderTable = {
				.StartAddress = m_raytracingPipeline->GetShaderTableByType(ShaderTableMiss)->GetAddress(),
				.SizeInBytes = m_raytracingPipeline->GetShaderTableByType(ShaderTableMiss)->GetSize(),
				.StrideInBytes = m_raytracingPipeline->GetShaderTableByType(ShaderTableMiss)->GetSize()
			},
			.HitGroupTable = {
				.StartAddress = m_raytracingPipeline->GetShaderTableByType(ShaderTableHitGroup)->GetAddress(),
				.SizeInBytes = m_raytracingPipeline->GetShaderTableByType(ShaderTableHitGroup)->GetSize(),
				.StrideInBytes = m_raytracingPipeline->GetShaderTableByType(ShaderTableHitGroup)->GetSize()
			},
			.CallableShaderTable = {},
			.Width = m_raytracedTextureWidth,
			.Height = m_raytracedTextureHeight,
			.Depth = 1
		};

		commandList->SetPipelineState1(m_raytracingPipeline->GetPipelineState());
		commandList->DispatchRays(&dispatchDesc);

		GraphicsUtils::UAVBarrier(commandList, m_testTexture->GetImageResource().Get());
	}

	void HardwareRaytracedDDGI::DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const
	{
		m_dataContainer.DebugDrawRaytracedImage(commandList, m_testTexture->GetSRV());
	}
}
