#include "RaytracedDDGIDataContainer.h"

#include "Components/MeshRenderer.h"
#include "ResourceManager/Material.h"
#include "SceneManager/GameObject.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	RaytracedDDGIDataContainer::RaytracedDDGIDataContainer(
		std::set<SharedMaterial*>& sceneSharedMaterials,
		uint32_t frameCount,
		DXGI_FORMAT mainColorFormat,
		DXGI_FORMAT depthFormat):
		m_sceneSharedMaterials(sceneSharedMaterials),
		m_raytracedProbesData(frameCount)
	{
		m_triangleDataBuffer = std::make_unique<DataBuffer<TrianglePayload>>(DATA_ARRAY_COUNT);
		m_meshDataBuffer = std::make_unique<DataBuffer<MeshData>>(DATA_ARRAY_COUNT);

		m_dispatcher = std::make_unique<ComputeDispatcher>();

		{
			// generate texture w = probesCount, h = DDGI_RAYS_COUNT
			m_probeIrradiancePipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
				{
					"shaders/sw_raytracing/ProbeIrradiance.hlsl"
				});
		}

		// Draw raytraced texture 
		{
			m_debugRaytracingTextureDrawGraphicsPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
				{
					"shaders/sw_raytracing/debugImageCompose.hlsl",
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_NEVER,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainColorFormat
					},
					1,
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		{
			m_debugSphereProbeMesh = ResourceManager::Get()->LoadResource<Mesh>(
				"models/DefaultSphere.obj:RootNode/DefaultSphere_root/pSphere1");
		}

		// Debug draw probes
		{
			m_debugDrawProbesGraphicsPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
				{
					"shaders/sw_raytracing/debugDrawProbes.hlsl",
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainColorFormat,
					},
					1,
					depthFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}
	}

	void RaytracedDDGIDataContainer::SetFrameData(uint32_t frameIndex, const ResourceView* skyboxTextureIndexDataView)
	{
		g_raytracedProbesData.skyboxTextureIndex = skyboxTextureIndexDataView->GetDescriptorIndex();
		m_raytracedProbesData.SetData(&g_raytracedProbesData, frameIndex);
	}

	void RaytracedDDGIDataContainer::UploadSceneData() const
	{
		TIME_PERF("Uploading scene data");

		uint32_t meshCount = 0;
		uint32_t trianglesCount = 0;
		for (auto const& sm : m_sceneSharedMaterials)
		{
			for (const auto& mr : sm->GetMeshRenderers())
			{
				if (!mr->IsStatic()) continue;
				const uint32_t meshTrianglesLength = mr->GetMesh()->GetIndexCount() / 3;

				for (uint32_t i = 0; i < meshTrianglesLength; i++, trianglesCount++)
				{
					m_triangleDataBuffer->GetLocalData()[trianglesCount] = TrianglePayload
					{
						.triangleIndex = i,
						.meshIndex = meshCount
					};
				}

				m_meshDataBuffer->GetLocalData()[meshCount] = MeshData{
					.materialIndex = mr->GetMaterial()->GetMaterialIndex(),
					.verticesIndex = mr->GetMesh()->GetVerticesBufferOffsetInBytes() / static_cast<uint32_t>(sizeof(Vertex)),
					.indicesIndex = mr->GetMesh()->GetIndicesBufferOffsetInBytes() / static_cast<uint32_t>(sizeof(Index)),
					.transformIndex = mr->GetGameObject().GetTransform().GetTransformIndex()
				};

				meshCount++;
			}
		}

		m_meshDataBuffer->UploadCpuData();
		m_triangleDataBuffer->UploadCpuData();
	}

	void RaytracedDDGIDataContainer::GenerateProbeIrradiance(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		const RenderTexture* shadedRenderTexture,
		const UAVGbuffer* gbuffer,
		const UAVTexture* probeIrradianceTexture,
		const UAVTexture* probeDepthTexture
	) const
	{
		// lightprobe data generation process
		{
			commandList->SetComputeRootSignature(m_probeIrradiancePipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_probeIrradiancePipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachView(commandList, m_probeIrradiancePipeline.get(), "shadedColorTexture", shadedRenderTexture->GetSRV());
			GraphicsUtils::AttachView(commandList, m_probeIrradiancePipeline.get(), "positionsTexture", gbuffer->GetPositionSRV());

			GraphicsUtils::AttachView(commandList, m_probeIrradiancePipeline.get(), "probeIrradianceTexture", probeIrradianceTexture->GetUAV());
			GraphicsUtils::AttachView(commandList, m_probeIrradiancePipeline.get(), "probeDepthTexture", probeDepthTexture->GetUAV());

			GraphicsUtils::AttachView(commandList, m_probeIrradiancePipeline.get(), "raytracedProbesData", m_raytracedProbesData.GetView(frameIndex));
		}
		commandList->Dispatch(g_raytracedProbesData.gridX, g_raytracedProbesData.gridY, g_raytracedProbesData.gridZ);

		GraphicsUtils::UAVBarrier(commandList, probeIrradianceTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, probeDepthTexture->GetImageResource().Get());
	}


	void RaytracedDDGIDataContainer::DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList, const ResourceView* texture) const
	{
		auto& sm = m_debugRaytracingTextureDrawGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsUtils::AttachView(commandList, sm.get(), "shadedColorTexture", texture);

		commandList->DrawInstanced(3, 1, 0, 0);
	}

	void RaytracedDDGIDataContainer::DebugDrawProbes(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		const ViewProjectionMatrixData* viewProjectionMatrixData,
		const ResourceView* probeIrradianceTexture) const
	{
		auto& sm = m_debugDrawProbesGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, m_debugSphereProbeMesh->GetVertexBufferView());
		commandList->IASetIndexBuffer(m_debugSphereProbeMesh->GetIndexBufferView());

		GraphicsUtils::ProcessEngineBindings(commandList, sm.get(), frameIndex, nullptr, viewProjectionMatrixData);

		GraphicsUtils::AttachView(commandList, sm.get(), "raytracedProbesData", m_raytracedProbesData.GetView(frameIndex));
		GraphicsUtils::AttachView(commandList, sm.get(), "irradianceTexture", probeIrradianceTexture);
		GraphicsUtils::AttachView(commandList, sm.get(), "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());

		const uint32_t instanceCount = g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * g_raytracedProbesData.gridZ;

		commandList->DrawInstanced(
			m_debugSphereProbeMesh->GetIndexCount(),
			instanceCount,
			0,
			0);
	}
}
