#include "RaytracedLightProbes.h"

#include "Common/HashDefs.h"
#include "ResourceManager/ResourceManager.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Material.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/Log.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	const AABB g_sceneAabb = {
		.min = glm::vec3(-40.0f, -3.0f, -25.0f),
		.max = glm::vec3(40.0f, 30.0f, 25.0f),
	};

	const RaytracedProbesData g_raytracedProbesData = {
		.gridMin = glm::vec3(-27, 1, -12),
		.cellSize = 4.1f,
		.gridX = 14,
		.gridY = 8,
		.gridZ = 6
	};

	uint32_t ExpandBits(uint32_t v)
	{
		v = (v * 0x00010001) & 0xFF0000FF;
		v = (v * 0x00000101) & 0x0F00F00F;
		v = (v * 0x00000011) & 0xC30C30C3;
		v = (v * 0x00000005) & 0x49249249;
		return v;
	}

	uint32_t Morton3D(float x, float y, float z)
	{
		x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
		y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
		z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
		uint32_t xx = ExpandBits(static_cast<uint32_t>(x));
		uint32_t yy = ExpandBits(static_cast<uint32_t>(y));
		uint32_t zz = ExpandBits(static_cast<uint32_t>(z));
		return xx * 4 + yy * 2 + zz;
	}

	void GetCentroidAndAABB(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3* centroid, AABB* aabb)
	{
		glm::vec3 min = glm::vec3(
			std::min(std::min(a.x, b.x), c.x) - 0.001f,
			std::min(std::min(a.y, b.y), c.y) - 0.001f,
			std::min(std::min(a.z, b.z), c.z) - 0.001f
		);
		glm::vec3 max = glm::vec3(
			std::max(std::max(a.x, b.x), c.x) + 0.001f,
			std::max(std::max(a.y, b.y), c.y) + 0.001f,
			std::max(std::max(a.z, b.z), c.z) + 0.001f
		);

		*centroid = (min + max) * 0.5f;
		*aabb = {
			.min = min,
			.max = max,
		};
	}

	glm::vec3 NormalizeCentroid(glm::vec3 centroid)
	{
		glm::vec3 ret = centroid;
		ret.x -= g_sceneAabb.min.x;
		ret.y -= g_sceneAabb.min.y;
		ret.z -= g_sceneAabb.min.z;
		ret.x /= (g_sceneAabb.max.x - g_sceneAabb.min.x);
		ret.y /= (g_sceneAabb.max.y - g_sceneAabb.min.y);
		ret.z /= (g_sceneAabb.max.z - g_sceneAabb.min.z);
		return ret;
	}

	RaytracedLightProbes::RaytracedLightProbes(
		std::set<SharedMaterial*>& sceneSharedMaterials,
		DXGI_FORMAT mainColorFormat,
		DXGI_FORMAT gBufferPositionsFormat,
		DXGI_FORMAT gBufferNormalsFormat,
		DXGI_FORMAT swapchainFormat,
		DXGI_FORMAT depthFormat,
		uint32_t width,
		uint32_t height) :
		m_mainColorFormat(mainColorFormat),
		m_gBufferPositionsFormat(gBufferPositionsFormat),
		m_gBufferNormalsFormat(gBufferNormalsFormat),
		m_swapchainFormat(swapchainFormat),
#if defined(CAMERA_TRACE)
		m_raytracedTextureWidth(width),
		m_raytracedTextureHeight(height),
#else
		m_raytracedTextureWidth(g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * g_raytracedProbesData.gridZ),
		m_raytracedTextureHeight(DDGI_RAYS_COUNT),
#endif

		m_sceneSharedMaterials(sceneSharedMaterials)
	{
		static_assert(sizeof(Triangle) == 128);
		static_assert(sizeof(AABB) == 32);

		m_raytracedProbesData.SetData(g_raytracedProbesData);

		m_keysBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleIndexBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleDataBuffer = std::make_unique<DataBuffer<Triangle>>(DATA_ARRAY_COUNT);
		m_triangleAABBBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);

		m_bvhDataBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);
		m_bvhLeafNodesBuffer = std::make_unique<DataBuffer<LeafNode>>(DATA_ARRAY_COUNT);
		m_bvhInternalNodesBuffer = std::make_unique<DataBuffer<InternalNode>>(DATA_ARRAY_COUNT);

		m_dispatcher = std::make_unique<ComputeDispatcher>();

		m_bufferSorter = std::make_unique<BufferSorter>(
			m_trianglesLength,
			m_keysBuffer.get(),
			m_triangleIndexBuffer.get(),
			m_dispatcher.get());

		m_bvhConstructor = std::make_unique<BVHConstructor>(
			m_keysBuffer.get(),
			m_triangleIndexBuffer.get(),
			m_triangleAABBBuffer.get(),
			m_bvhInternalNodesBuffer.get(),
			m_bvhLeafNodesBuffer.get(),
			m_bvhDataBuffer.get(),
			m_dispatcher.get(),
			&m_bvhConstructionData
		);

		// Raytracing
		{
			m_gbuffer = std::make_unique<UAVGbuffer>(m_raytracedTextureWidth, m_raytracedTextureHeight);
			m_shadedRenderTexture = std::make_unique<RenderTexture>(
				m_raytracedTextureWidth, m_raytracedTextureHeight,
				m_mainColorFormat,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT);

			m_probeIrradianceTexture = std::make_unique<UAVTexture>(
				g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * (DDGI_PROBE_DATA_RESOLUTION + 2),
				g_raytracedProbesData.gridZ * (DDGI_PROBE_DATA_RESOLUTION + 2),
				DXGI_FORMAT_R11G11B10_FLOAT,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT
			);

			m_probeDepthTexture = std::make_unique<UAVTexture>(
				g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * (DDGI_PROBE_DATA_RESOLUTION + 2),
				g_raytracedProbesData.gridZ * (DDGI_PROBE_DATA_RESOLUTION + 2),
				DXGI_FORMAT_R16G16_FLOAT,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT
			);


			{
				// generate texture w = probesCount, h = DDGI_RAYS_COUNT
				const GUID raytracingShaderGuid = GUID::StringToGuid("b24e90ac-fcfa-4754-b0e5-8553b19e27ca"); //shaders/raytracing/Raytracing.hlsl
				const GUID raytracingPipelineGuid = GUID::Random();

				m_raytracingPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
					raytracingPipelineGuid,
					{
						raytracingShaderGuid,
						D3D_SHADER_MODEL_6_5
					});
			}

			{
				// generate texture w = probesCount, h = DDGI_RAYS_COUNT
				const GUID pipelineIrradianceShaderGuid = GUID::StringToGuid("1d69c9ec-de6a-4fff-96ea-3a68808ca8f7"); //shaders/raytracing/ProbeIrradiance.hlsl
				const GUID pipelineIrradiancePipelineGuid = GUID::Random();

				m_probeIrradiancePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
					pipelineIrradiancePipelineGuid,
					{
						pipelineIrradianceShaderGuid
					});
			}
		}

		// Draw raytraced texture 
		{
			const GUID debugImageComposerShaderGuid = GUID::StringToGuid("cc8de13c-0510-4842-99f5-de2327aa95d4"); // shaders/raytracing/debugImageCompose.hlsl
			const GUID debugImageComposerSharedMaterialGuid = GUID::Random();

			m_debugRaytracingTextureDrawGraphicsPipeline = ResourceManager::Get()->LoadResource<
				GraphicsPipeline, GraphicsPipelineArgs>(
				debugImageComposerSharedMaterialGuid,
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
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Gizmo AABB draw 
		{
			const GUID gizmoAABBDrawerShaderGuid = GUID::StringToGuid("a231c467-dc15-4753-a3db-8888efc73c1a");
			// shaders/raytracing/gizmoAABBDrawer.hlsl
			const GUID gizmoAABBDrawerSharedMaterialGuid = GUID::Random();

			m_debugGizmoAABBDrawerGraphicsPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
				gizmoAABBDrawerSharedMaterialGuid,
				{
					gizmoAABBDrawerShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_NEVER,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						swapchainFormat
					},
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
				});
		}

		{
			m_debugSphereProbeMesh = ResourceManager::Get()->LoadResource<Mesh>(GUID::StringToGuid("b7d27f1a-006b-41fa-b10b-01b212ebfebe"));
		}

		// Debug draw probes
		{
			const GUID drawProbesShaderGuid = GUID::StringToGuid("8757d834-8dd7-4858-836b-bb6a4eb6fea0"); //shaders/raytracing/debugDrawProbes.hlsl
			const GUID drawProbesSharedMaterialGuid = GUID::Random();

			m_debugDrawProbesGraphicsPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
				drawProbesSharedMaterialGuid,
				{
					drawProbesShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					false,
					false,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						swapchainFormat,
						swapchainFormat,
						swapchainFormat,
					},
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}
	}

	void RaytracedLightProbes::UploadSceneData()
	{
		TIME_PERF("Uploading scene data")

		for (auto const& sm : m_sceneSharedMaterials)
		{
			for (const auto& mr : sm->GetMeshRenderers())
			{
				if (!mr->IsStatic()) continue;

				const uint32_t meshTrianglesLength = mr->GetMesh()->GetIndexCount() / 3;

				const Vertex* vertices = mr->GetMesh()->GetVertices();
				const uint32_t* indices = mr->GetMesh()->GetIndices();

				for (uint32_t i = 0; i < meshTrianglesLength; i++, m_trianglesLength++)
				{
					glm::vec3 a = vertices[indices[i * 3 + 0]].pos;
					glm::vec3 b = vertices[indices[i * 3 + 1]].pos;
					glm::vec3 c = vertices[indices[i * 3 + 2]].pos;

					glm::vec3 centroid;
					AABB aabb = {};

					GetCentroidAndAABB(a, b, c, &centroid, &aabb);
					centroid = NormalizeCentroid(centroid);
					const uint32_t mortonCode = Morton3D(centroid.x, centroid.y, centroid.z);
					m_keysBuffer->GetLocalData()[m_trianglesLength] = mortonCode;
					m_triangleIndexBuffer->GetLocalData()[m_trianglesLength] = m_trianglesLength;
					m_triangleDataBuffer->GetLocalData()[m_trianglesLength] = Triangle
					{
						.a = a,
						.b = b,
						.c = c,
						.a_uv = vertices[indices[i * 3 + 0]].texCoord,
						.b_uv = vertices[indices[i * 3 + 1]].texCoord,
						.c_uv = vertices[indices[i * 3 + 2]].texCoord,
						.materialIndex = mr->GetMaterial()->GetMaterialIndex(),
						.objectIndex = mr->GetGameObject().GetTransformIndex(),
						.a_normal = vertices[indices[i * 3 + 0]].normal,
						.b_normal = vertices[indices[i * 3 + 1]].normal,
						.c_normal = vertices[indices[i * 3 + 2]].normal,
					};
					m_triangleAABBBuffer->GetLocalData()[m_trianglesLength] = aabb;
				}
			}
		}

		m_keysBuffer->UploadCpuData();
		m_triangleIndexBuffer->UploadCpuData();
		m_triangleDataBuffer->UploadCpuData();
		m_triangleAABBBuffer->UploadCpuData();
		m_bvhConstructionData.SetData({.trianglesCount = m_trianglesLength});
	}

	void RaytracedLightProbes::PrepareBVH() const
	{
		Logger::LogFormat("Triangles length %d\n", m_trianglesLength);

		TIME_PERF("Prepare Scene BVH")

		m_bufferSorter->Sort();

		{
			// Update keys array. Now we guarantee all the elements are unique
			m_keysBuffer->ReadbackGpuData();

			uint32_t* keysArray = m_keysBuffer->GetLocalData();

			uint32_t newCurrentValue = 0;
			uint32_t oldCurrentValue = keysArray[0];
			keysArray[0] = newCurrentValue;

			for (uint32_t i = 1; i < m_trianglesLength; i++)
			{
				newCurrentValue += std::max(keysArray[i] - oldCurrentValue, 1u);
				oldCurrentValue = keysArray[i];
				keysArray[i] = newCurrentValue;
			}

			m_keysBuffer->UploadCpuData();
		}

		m_bvhConstructor->ConstructTree();
		m_bvhConstructor->ConstructBVH();
	}

	void RaytracedLightProbes::ProcessRaytracing(
		ID3D12GraphicsCommandList* commandList,
		uint32_t frameIndex,
		ViewProjectionMatrixData* data,
		ResourceView* skyboxTextureIndexDataView) const
	{
		// Raytracing process
		{
			commandList->SetComputeRootSignature(m_raytracingPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_raytracingPipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "colorTexture", m_gbuffer->GetColorUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "normalsTexture", m_gbuffer->GetNormalsUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "positionTexture", m_gbuffer->GetPositionUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "textures", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "materials", EngineMaterialProvider::Get()->GetMaterialsDataView());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "sortedTriangleIndices", m_triangleIndexBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleAABB", m_triangleAABBBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "internalNodes", m_bvhInternalNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "leafNodes", m_bvhLeafNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "bvhData", m_bvhDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleData", m_triangleDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());
#if !defined(CAMERA_TRACE)
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "raytracedProbesData", m_raytracedProbesData.GetView());
#endif
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "skyboxTextureIndex", skyboxTextureIndexDataView);

			GraphicsUtils::ProcessEngineBindings(commandList, frameIndex, m_raytracingPipeline->GetEngineBindings(),
			                                     nullptr, data);
		}
#if defined(CAMERA_TRACE)
		commandList->Dispatch((m_width / 32) + 1, (m_height / 32) + 1, 1);
#else
		commandList->Dispatch(g_raytracedProbesData.gridX, g_raytracedProbesData.gridY, g_raytracedProbesData.gridZ);
#endif

		m_gbuffer->BarrierToRead(commandList);
	}

	void RaytracedLightProbes::GenerateProbeIrradiance(ID3D12GraphicsCommandList* commandList) const
	{
		// lightprobe data generation process
		{
			commandList->SetComputeRootSignature(m_probeIrradiancePipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_probeIrradiancePipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, m_probeIrradiancePipeline, "shadedColorTexture", m_shadedRenderTexture->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_probeIrradiancePipeline, "positionsTexture", m_gbuffer->GetPositionSRV());

			GraphicsUtils::AttachViewToCompute(commandList, m_probeIrradiancePipeline, "probeIrradianceTexture", m_probeIrradianceTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_probeIrradiancePipeline, "probeDepthTexture", m_probeDepthTexture->GetUAV());

			GraphicsUtils::AttachViewToCompute(commandList, m_probeIrradiancePipeline, "raytracedProbesData", m_raytracedProbesData.GetView());
		}
		commandList->Dispatch(g_raytracedProbesData.gridX, g_raytracedProbesData.gridY, g_raytracedProbesData.gridZ);

		GraphicsUtils::UAVBarrier(commandList, m_probeIrradianceTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, m_probeDepthTexture->GetImageResource().Get());
	}

	void RaytracedLightProbes::DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const
	{
		auto sm = m_debugRaytracingTextureDrawGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#if defined(CAMERA_TRACE)
		GraphicsUtils::AttachViewToGraphics(commandList, sm, "raytracingTexture", m_shadedRenderTexture->GetSRV());
#else
		GraphicsUtils::AttachViewToGraphics(commandList, sm, "raytracingTexture", m_probeIrradianceTexture->GetSRV());
#endif

		commandList->DrawInstanced(3, 1, 0, 0);
	}

	void RaytracedLightProbes::DebugDrawAABBGizmo(ID3D12GraphicsCommandList* commandList,
	                                              const ViewProjectionMatrixData* viewProjectionMatrixData) const
	{
		auto sm = m_debugGizmoAABBDrawerGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);


		commandList->SetGraphicsRoot32BitConstants(1, sizeof(ViewProjectionMatrixData) / 4,
		                                           viewProjectionMatrixData,
		                                           0);

		GraphicsUtils::AttachViewToGraphics(commandList, sm, "BVHData", m_bvhDataBuffer->GetSRV());

		commandList->DrawInstanced(
			24,
			DATA_ARRAY_COUNT,
			0, 0);
	}

	void RaytracedLightProbes::DebugDrawProbes(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionMatrixData) const
	{
		auto sm = m_debugDrawProbesGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->IASetVertexBuffers(0, 1, m_debugSphereProbeMesh->GetVertexBufferView());
		commandList->IASetIndexBuffer(m_debugSphereProbeMesh->GetIndexBufferView());

		GraphicsUtils::ProcessEngineBindings(commandList, frameIndex, sm->GetEngineBindings(), nullptr, viewProjectionMatrixData);

		GraphicsUtils::AttachViewToGraphics(commandList, sm, "raytracedProbesData", m_raytracedProbesData.GetView());
		GraphicsUtils::AttachViewToGraphics(commandList, sm, "irradianceTexture", m_probeIrradianceTexture->GetSRV());
		GraphicsUtils::AttachViewToGraphics(commandList, sm, "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());

		const uint32_t instanceCount = g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * g_raytracedProbesData.gridZ;

		commandList->DrawInstanced(
			m_debugSphereProbeMesh->GetIndexCount(),
			instanceCount,
			0,
			0);
	}
}
