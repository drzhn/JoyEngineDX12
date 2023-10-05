#include "ClusteredLightSystem.h"

#include <cmath>

#include "Components/Camera.h"
#include "Components/MeshRenderer.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Mesh.h"
#include "SceneManager/GameObject.h"
#include "Utils/GraphicsUtils.h"

#define DIRECTIONAL_SHADOWMAP_SIZE 2048

namespace JoyEngine
{
	bool SphereCubeIntersection(glm::vec3 cubeMin, glm::vec3 cubeMax, glm::vec3 sphereCenter, float sphereRadius)
	{
		float dmin = 0;
		float r2 = pow(sphereRadius, 2.f);
		for (int i = 0; i < 3; i++)
		{
			if (sphereCenter[i] < cubeMin[i])
				dmin += pow(sphereCenter[i] - cubeMin[i], 2.f);
			else if (sphereCenter[i] > cubeMax[i])
				dmin += pow(sphereCenter[i] - cubeMax[i], 2.f);
		}

		if (dmin <= r2)
		{
			return true;
		}
		return false;
	}

	inline glm::vec4 ToVec4(glm::vec3 vec3)
	{
		return {vec3.x, vec3.y, vec3.z, 1};
	}

	ClusteredLightSystem::ClusteredLightSystem(const uint32_t frameCount) :
		m_frameCount(frameCount),
		m_camera(nullptr),
		m_directionalLightData(),
		m_lightDataPool(frameCount),
		m_clusterEntryData(frameCount),
		m_clusterItemData(frameCount)
	{
	}

	void ClusteredLightSystem::Update(const uint32_t frameIndex)
	{
		ASSERT(m_camera != nullptr);

		m_directionalLightDataBuffer->SetData(&m_directionalLightData, frameIndex);


		// update light info (intensity, color, etc)
		{
			auto& buffer = m_lightDataPool.GetDynamicBuffer();

			LightData* dataPtr = buffer.GetPtr(frameIndex);
			const auto& dataArray = m_lightDataPool.GetDataArray();
			for (uint32_t i = 0; i < dataArray.size(); ++i)
			{
				dataPtr->data[i] = dataArray[i];
			}

		}

		const float cameraNear = m_camera->GetNear();
		const float cameraFar = m_camera->GetFar();
		const float cameraAspect = m_camera->GetAspect();
		const float cameraFowRad = m_camera->GetFovRadians();

		const float distance = cameraFar - cameraNear;
		const float logDistance = log2(distance + 1);

		const glm::mat4 cameraViewMatrix = m_camera->GetViewMatrix();

		auto GetCubeVertex = [&](int x, int y, int z)
		{
			const float nearZ = (pow(2.0f, logDistance / NUM_CLUSTERS_Z * z) - 1 + cameraNear);
			const float nearH = 2 * nearZ * tan(cameraFowRad / 2.f);
			const float nearW = cameraAspect * nearH;
			const float cubeX = x * nearW / NUM_CLUSTERS_X - nearW / 2;
			const float cubeY = y * nearH / NUM_CLUSTERS_Y - nearH / 2;

			return glm::vec3(cubeX, cubeY, nearZ);
		};

		for (auto& pair : m_lights) // TODO make it in parallel
		{
			glm::vec4 sphereCenter = ToVec4(pair.first->GetGameObject().GetTransform()->GetPosition());
			sphereCenter = cameraViewMatrix * sphereCenter;
			pair.second = sphereCenter;
		}

		for (int z = 0; z < NUM_CLUSTERS_Z; z++) // TODO make it in parallel
		{
			for (int x = 0; x < NUM_CLUSTERS_X; x++)
			{
				for (int y = 0; y < NUM_CLUSTERS_Y; y++)
				{
					glm::vec3 cubePoints[8];

					cubePoints[0] = GetCubeVertex(x + 0, y + 0, z + 0);
					cubePoints[1] = GetCubeVertex(x + 0, y + 0, z + 1);
					cubePoints[2] = GetCubeVertex(x + 0, y + 1, z + 0);
					cubePoints[3] = GetCubeVertex(x + 0, y + 1, z + 1);
					cubePoints[4] = GetCubeVertex(x + 1, y + 0, z + 0);
					cubePoints[5] = GetCubeVertex(x + 1, y + 0, z + 1);
					cubePoints[6] = GetCubeVertex(x + 1, y + 1, z + 0);
					cubePoints[7] = GetCubeVertex(x + 1, y + 1, z + 1);

					glm::vec3 cubeMin = cubePoints[0];
					glm::vec3 cubeMax = cubePoints[0];

					for (int i = 1; i < 8; i++)
					{
						cubeMin = glm::min(cubePoints[i], cubeMin);
						cubeMax = glm::max(cubePoints[i], cubeMax);
					}

					int currentLight = 0;
					const int clusterStartingPoint =
						y * LIGHTS_PER_CLUSTER +
						x * LIGHTS_PER_CLUSTER * NUM_CLUSTERS_Y +
						z * LIGHTS_PER_CLUSTER * NUM_CLUSTERS_Y * NUM_CLUSTERS_X;

					for (const auto& light : m_lights)
					{
						uint32_t lightIndex = light.first->GetIndex();

						if (SphereCubeIntersection(cubeMin, cubeMax, light.second, m_lightDataPool.GetElem(lightIndex).radius))
						{
							ASSERT(currentLight < LIGHTS_PER_CLUSTER);
							m_clusterLightIndices[clusterStartingPoint + currentLight] = lightIndex;
							currentLight++;
						}
					}

					for (int i = currentLight; i < LIGHTS_PER_CLUSTER; i++)
					{
						m_clusterLightIndices[clusterStartingPoint + i] = -1;
					}
				}
			}
		}

		// Convolution
		ClusterEntryData* entryDataPtr = m_clusterEntryData.GetPtr(frameIndex);

		ClusterItemData* itemDataPtr = m_clusterItemData.GetPtr(frameIndex);

		uint32_t currentOffset = 0;
		for (int z = 0; z < NUM_CLUSTERS_Z; z++)
		{
			for (int x = 0; x < NUM_CLUSTERS_X; x++)
			{
				for (int y = 0; y < NUM_CLUSTERS_Y; y++)
				{
					const int clusterIndex =
						y +
						x * NUM_CLUSTERS_Y +
						z * NUM_CLUSTERS_Y * NUM_CLUSTERS_X;

					uint32_t numLight = 0;
					uint32_t prevOffset = currentOffset;

					for (int i = 0; i < LIGHTS_PER_CLUSTER; i++)
					{
						if (m_clusterLightIndices[clusterIndex * LIGHTS_PER_CLUSTER + i] == -1)
						{
							break;
						}
						itemDataPtr->data[currentOffset].lightIndex = m_clusterLightIndices[clusterIndex * LIGHTS_PER_CLUSTER + i];
						numLight++;
						currentOffset++;
						ASSERT(currentOffset < CLUSTER_ITEM_DATA_SIZE); // Pls, increase CLUSTER_ITEM_DATA_SIZE
					}

					entryDataPtr->data[clusterIndex] = {
						.offset = prevOffset,
						.numLight = numLight
					};
				}
			}
		}

	}

	void ClusteredLightSystem::RenderDirectionalShadows(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		SharedMaterial* gBufferSharedMaterial) const
	{
		GraphicsUtils::BeginDebugEvent(commandList, "Directional Light");

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
				sm->GetGraphicsPipeline(),
				frameIndex,
				mr->GetGameObject().GetTransformIndexPtr(),
				&viewProjectionMatrixData);

			commandList->DrawIndexedInstanced(
				mr->GetMesh()->GetIndexCount(),
				1,
				0, 0, 0);
		}

		GraphicsUtils::Barrier(commandList, m_directionalShadowmap->GetImageResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);

		GraphicsUtils::EndDebugEvent(commandList);
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
		m_lights.insert({ light, glm::vec4() });
		return m_lightDataPool.Allocate();
	}

	void ClusteredLightSystem::UnregisterLight(LightBase* light)
	{
		ASSERT(m_lights.contains(light));
		m_lights.erase(light);
		m_lightDataPool.Free(light->GetIndex());
	}
}
