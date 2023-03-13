#include "ClusteredLightSystem.h"

#include "Components/MeshRenderer.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Mesh.h"
#include "SceneManager/GameObject.h"
#include "Utils/GraphicsUtils.h"

#define DIRECTIONAL_SHADOWMAP_SIZE 2048

namespace JoyEngine
{
	ClusteredLightSystem::ClusteredLightSystem(const uint32_t frameCount) :
		m_frameCount(frameCount),
		m_camera(nullptr),
		m_directionalLightData(),
		m_lightDataPool(frameCount)
	{
	}

	void ClusteredLightSystem::Update(const uint32_t frameIndex)
	{
		ASSERT(m_camera != nullptr);

		m_directionalLightDataBuffer->SetData(&m_directionalLightData, frameIndex);

		{
			auto& buffer = m_lightDataPool.GetDynamicBuffer();
			buffer.Lock(frameIndex);

			LightData* dataPtr = buffer.GetPtr();
			const auto& dataArray = m_lightDataPool.GetDataArray();
			for (uint32_t i = 0; i < dataArray.size(); ++i)
			{
				dataPtr->data[i] = dataArray[i];
			}

			buffer.Unlock();
		}
	}

	void ClusteredLightSystem::RenderDirectionalShadows(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		SharedMaterial* gBufferSharedMaterial)
	{
		GraphicsUtils::Barrier(commandList, m_directionalShadowmap->GetImageResource().Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
		                       D3D12_RESOURCE_STATE_DEPTH_WRITE);

		GraphicsUtils::SetViewportAndScissor(commandList, m_directionalShadowmap->GetWidth(), m_directionalShadowmap->GetHeight());

		const auto shadowMapHandle = m_directionalShadowmap->GetDSV()->GetCPUHandle();

		commandList->OMSetRenderTargets(
			0,
			nullptr,
			FALSE, &shadowMapHandle);

		commandList->ClearDepthStencilView(shadowMapHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		const auto sm = EngineMaterialProvider::Get()->GetShadowProcessingSharedMaterial();
		commandList->SetPipelineState(sm->GetGraphicsPipeline()->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetGraphicsPipeline()->GetRootSignature().Get());

		const ViewProjectionMatrixData viewProjectionMatrixData = {
			.view = m_directionalLightData.view,
			.proj = m_directionalLightData.proj,
		};

		for (const auto& mr : gBufferSharedMaterial->GetMeshRenderers())
		{
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, mr->GetMesh()->GetVertexBufferView());
			commandList->IASetIndexBuffer(mr->GetMesh()->GetIndexBufferView());

			GraphicsUtils::ProcessEngineBindings(
				commandList,
				frameIndex,
				sm->GetGraphicsPipeline()->GetEngineBindings(),
				mr->GetGameObject().GetTransformIndexPtr(),
				&viewProjectionMatrixData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
		}

		GraphicsUtils::Barrier(commandList, m_directionalShadowmap->GetImageResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);
	}


	uint32_t ClusteredLightSystem::RegisterDirectionalLight()
	{
		m_directionalLightData.bias = 0.001f;
		m_directionalLightData.shadowmapSize = DIRECTIONAL_SHADOWMAP_SIZE;

		m_directionalShadowmap = std::make_unique<DepthTexture>(
			DIRECTIONAL_SHADOWMAP_SIZE,
			DIRECTIONAL_SHADOWMAP_SIZE,
			DXGI_FORMAT_R32_TYPELESS,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);

		m_directionalLightDataBuffer = std::make_unique<DynamicCpuBuffer<DirectionalLightInfo>>(m_frameCount);

		return 0;
	}

	void ClusteredLightSystem::UnregisterDirectionalLight()
	{
		// empty
	}

	uint32_t ClusteredLightSystem::RegisterLight(LightBase* light)
	{
		ASSERT(!m_lights.contains(light));
		m_lights.insert(light);
		return m_lightDataPool.Allocate();
	}

	void ClusteredLightSystem::UnregisterLight(LightBase* light)
	{
		ASSERT(m_lights.contains(light));
		m_lights.erase(light);
		m_lightDataPool.Free(light->GetIndex());
	}
}
