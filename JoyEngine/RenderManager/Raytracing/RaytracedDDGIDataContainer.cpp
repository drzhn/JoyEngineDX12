#include "RaytracedDDGIDataContainer.h"

#include "Components/MeshRenderer.h"
#include "RenderManager/RenderManager.h"
#include "ResourceManager/Material.h"
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
		m_triangleDataBuffer = std::make_unique<DataBuffer<Triangle>>(DATA_ARRAY_COUNT);
		m_dispatcher = std::make_unique<ComputeDispatcher>();

		// Draw raytraced texture 
		{
			const GUID debugImageComposerShaderGuid = GUID::StringToGuid("cc8de13c-0510-4842-99f5-de2327aa95d4"); // shaders/raytracing/debugImageCompose.hlsl

			m_debugRaytracingTextureDrawGraphicsPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
				{
					debugImageComposerShaderGuid,
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
			m_debugSphereProbeMesh = ResourceManager::Get()->LoadResource<Mesh>(GUID::StringToGuid("b7d27f1a-006b-41fa-b10b-01b212ebfebe"));
		}

		// Debug draw probes
		{
			const GUID drawProbesShaderGuid = GUID::StringToGuid("8757d834-8dd7-4858-836b-bb6a4eb6fea0"); //shaders/raytracing/debugDrawProbes.hlsl

			m_debugDrawProbesGraphicsPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
				{
					drawProbesShaderGuid,
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

		uint32_t staticMeshCount = 0;
		uint32_t m_trianglesLength = 0;
		for (auto const& sm : m_sceneSharedMaterials)
		{
			for (const auto& mr : sm->GetMeshRenderers())
			{
				if (!mr->IsStatic()) continue;
				staticMeshCount++;
				const uint32_t meshTrianglesLength = mr->GetMesh()->GetIndexCount() / 3;

				for (uint32_t i = 0; i < meshTrianglesLength; i++, m_trianglesLength++)
				{
					m_triangleDataBuffer->GetLocalData()[m_trianglesLength] = Triangle
					{
						.materialIndex = mr->GetMaterial()->GetMaterialIndex(),
						.verticesIndex = mr->GetMesh()->GetVertexSRV()->GetDescriptorIndex(),
						.indicesIndex = mr->GetMesh()->GetIndexSRV()->GetDescriptorIndex(),
						.triangleIndex = i
					};
				}
			}
		}

		m_triangleDataBuffer->UploadCpuData();
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
