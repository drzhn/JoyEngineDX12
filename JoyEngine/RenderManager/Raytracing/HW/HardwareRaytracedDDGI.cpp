#include "HardwareRaytracedDDGI.h"

#include "CommonEngineStructs.h"
#include "Common/HashDefs.h"
#include "GraphicsManager/GraphicsManager.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"

namespace JoyEngine
{
	HardwareRaytracedDDGI::HardwareRaytracedDDGI()
	{
		m_dispatcher = std::make_unique<ComputeDispatcher>();
		m_testTexture = std::make_unique<UAVTexture>(
			1024,
			1024,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);

		float border = 0.1f;

		m_screenParamsBuffer.SetData({
			.viewport = {-1.0f, -1.0f, 1.0f, 1.0f},
			.stencil = {
				-1 + border, -1 + border,
				1.0f - border, 1 - border
			}
		});
		m_testColorBuffer.SetData({{1, 0, 0, 1}});

		m_raytracingPipeline = std::make_unique<RaytracingPipeline>(RaytracingPipelineArgs{
			GUID::StringToGuid("b2597599-94ef-43ed-abd8-46d3adbb75d4")
		});

		uint16_t indices[] =
		{
			0, 1, 2
		};

		float depthValue = 1.0;
		float offset = 0.7f;

		Vertex vertices[] =
		{
			// The sample raytraces in screen space coordinates.
			// Since DirectX screen space coordinates are right handed (i.e. Y axis points down).
			// Define the vertices in counter clockwise order ~ clockwise in left handed.
			{{0, -offset, depthValue}},
			{{-offset, offset, depthValue}},
			{{offset, offset, depthValue}}
		};

		m_testIndexBuffer = std::make_unique<Buffer>(
			sizeof(indices),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_FLAG_NONE
		);

		m_testIndexBuffer->SetCPUData(indices, 0, sizeof(indices));

		m_testVertexBuffer = std::make_unique<Buffer>(
			sizeof(vertices),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_UPLOAD,
			D3D12_RESOURCE_FLAG_NONE
		);

		m_testVertexBuffer->SetCPUData(vertices, 0, sizeof(vertices));

		// Acceleration structures

		D3D12_RAYTRACING_GEOMETRY_DESC raytracingGeometryDesc = {
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
			.Triangles = {
				.Transform3x4 = 0,
				.IndexFormat = DXGI_FORMAT_R16_UINT,
				.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
				.IndexCount = sizeof(indices) / sizeof(uint16_t),
				.VertexCount = sizeof(vertices) / sizeof(Vertex),
				.IndexBuffer = m_testIndexBuffer->GetBufferResource()->GetGPUVirtualAddress(),
				.VertexBuffer =
				{
					.StartAddress = m_testVertexBuffer->GetBufferResource()->GetGPUVirtualAddress(),
					.StrideInBytes = sizeof(Vertex)
				}
			}
		};

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

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
		GraphicsManager::Get()->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
		ASSERT(topLevelPrebuildInfo.ScratchDataSizeInBytes > 0);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE,
			.NumDescs = 1,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.pGeometryDescs = &raytracingGeometryDesc
		};
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
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

		auto commandList = m_dispatcher->GetCommandList();

		commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
		GraphicsUtils::UAVBarrier(commandList, m_accelerationBottom->GetBuffer()->GetBufferResource().Get());
		commandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

		m_dispatcher->ExecuteAndWait();
	}

	void HardwareRaytracedDDGI::ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const
	{
		commandList->SetComputeRootSignature(m_raytracingPipeline->GetGlobalInputContainer()->GetRootSignature().Get());

		commandList->SetComputeRootShaderResourceView(
			m_raytracingPipeline->GetGlobalInputContainer()->GetBindingIndexByHash(strHash("g_SceneAccelerationStructure")),
			m_accelerationTop->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress());

		GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "g_OutputRenderTarget", m_testTexture->GetUAV());
		GraphicsUtils::AttachView(m_raytracingPipeline.get(), ShaderTableRaygen, "screenParams", m_screenParamsBuffer.GetView());
		GraphicsUtils::AttachView(m_raytracingPipeline.get(), ShaderTableHitGroup, "hitColor", m_testColorBuffer.GetView());

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
			.Width = 1024,
			.Height = 1024,
			.Depth = 1
		};

		commandList->SetPipelineState1(m_raytracingPipeline->GetPipelineState());
		commandList->DispatchRays(&dispatchDesc);

		GraphicsUtils::UAVBarrier(commandList, m_testTexture->GetImageResource().Get());
	}
}
