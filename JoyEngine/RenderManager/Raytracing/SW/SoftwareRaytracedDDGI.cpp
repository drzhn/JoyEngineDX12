#include "SoftwareRaytracedDDGI.h"

#include "Common/HashDefs.h"
#include "ResourceManager/ResourceManager.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "SceneManager/GameObject.h"
#include "SceneManager/Transform.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/Log.h"
#include "Utils/TimeCounter.h"

#include "RenderManager/Raytracing/RaytracedDDGIDataContainer.h"

namespace JoyEngine
{
	const AABB g_sceneAabb = {
		.min = jmath::vec3(-50.0f, -30.0f, -50.0f),
		.max = jmath::vec3(50.0f, 50.0f, 50.0f),
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

	void GetCentroidAndAABB(jmath::vec3 a, jmath::vec3 b, jmath::vec3 c, jmath::vec3* centroid, AABB* aabb)
	{
		jmath::vec3 min = jmath::vec3(
			std::min(std::min(a.x, b.x), c.x) - 0.001f,
			std::min(std::min(a.y, b.y), c.y) - 0.001f,
			std::min(std::min(a.z, b.z), c.z) - 0.001f
		);
		jmath::vec3 max = jmath::vec3(
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

	jmath::vec3 NormalizeCentroid(jmath::vec3 centroid)
	{
		jmath::vec3 ret = centroid;
		ret.x -= g_sceneAabb.min.x;
		ret.y -= g_sceneAabb.min.y;
		ret.z -= g_sceneAabb.min.z;
		ret.x /= (g_sceneAabb.max.x - g_sceneAabb.min.x);
		ret.y /= (g_sceneAabb.max.y - g_sceneAabb.min.y);
		ret.z /= (g_sceneAabb.max.z - g_sceneAabb.min.z);
		return ret;
	}

	SoftwareRaytracedDDGI::SoftwareRaytracedDDGI(
		const RaytracedDDGIDataContainer& dataContainer,
		DXGI_FORMAT mainColorFormat,
		DXGI_FORMAT swapchainFormat,
		uint32_t width,
		uint32_t height):
		m_dataContainer(dataContainer),
#if defined(CAMERA_TRACE)
		m_raytracedTextureWidth(width),
		m_raytracedTextureHeight(height)
#else
		m_raytracedTextureWidth(g_raytracedProbesData.gridX * g_raytracedProbesData.gridY * g_raytracedProbesData.gridZ),
		m_raytracedTextureHeight(DDGI_RAYS_COUNT)
#endif
	{
		static_assert(sizeof(TrianglePayload) == 16);
		static_assert(sizeof(AABB) == 32);

		m_keysBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleIndexBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleAABBBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);

		m_bvhDataBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);
		m_bvhLeafNodesBuffer = std::make_unique<DataBuffer<LeafNode>>(DATA_ARRAY_COUNT);
		m_bvhInternalNodesBuffer = std::make_unique<DataBuffer<InternalNode>>(DATA_ARRAY_COUNT);

		m_bufferSorter = std::make_unique<BufferSorter>(
			m_trianglesLength,
			m_keysBuffer.get(),
			m_triangleIndexBuffer.get(),
			m_dataContainer.GetDispatcher());

		m_bvhConstructor = std::make_unique<BVHConstructor>(
			m_keysBuffer.get(),
			m_triangleIndexBuffer.get(),
			m_triangleAABBBuffer.get(),
			m_bvhInternalNodesBuffer.get(),
			m_bvhLeafNodesBuffer.get(),
			m_bvhDataBuffer.get(),
			m_dataContainer.GetDispatcher(),
			&m_bvhConstructionData
		);

		// Raytracing texture resources
		// TODO move to Data container
		{
			m_gbuffer = std::make_unique<UAVGbuffer>(m_raytracedTextureWidth, m_raytracedTextureHeight);
			m_shadedRenderTexture = std::make_unique<RenderTexture>(
				m_raytracedTextureWidth, m_raytracedTextureHeight,
				mainColorFormat,
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

				m_raytracingPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/sw_raytracing/Raytracing.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
		}

		// Gizmo AABB draw 
		{
			m_debugGizmoAABBDrawerGraphicsPipeline = std::make_unique<GraphicsPipeline>(GraphicsPipelineArgs
				{
					"shaders/sw_raytracing/gizmoAABBDrawer.hlsl",
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
					1,
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
				});
		}
	}


	void SoftwareRaytracedDDGI::UploadSceneData()
	{
		{
			TIME_PERF("Uploading software DDGI scene data")

			for (auto const& sm : m_dataContainer.GetSceneSharedMaterials())
			{
				for (const auto& mr : sm->GetMeshRenderers())
				{
					if (!mr->IsStatic()) continue;

					const uint32_t meshTrianglesLength = mr->GetMesh()->GetIndexCount() / 3;

					const Vertex* vertices = mr->GetMesh()->GetVertices();
					const uint32_t* indices = mr->GetMesh()->GetIndices();

					const auto& modelMatrix = mr->GetGameObject().GetTransform().GetModelMatrix();

					for (uint32_t i = 0; i < meshTrianglesLength; i++, m_trianglesLength++)
					{
						jmath::vec3 a = jmath::toVec3(jmath::mul(
							modelMatrix,
							jmath::loadPosition(vertices[indices[i * 3 + 0]].pos)));
						jmath::vec3 b = jmath::toVec3(jmath::mul(
							modelMatrix,
							jmath::loadPosition(vertices[indices[i * 3 + 1]].pos)));
						jmath::vec3 c = jmath::toVec3(jmath::mul(
							modelMatrix,
							jmath::loadPosition(vertices[indices[i * 3 + 2]].pos)));

						jmath::vec3 centroid;
						AABB aabb = {};

						GetCentroidAndAABB(a, b, c, &centroid, &aabb);
						centroid = NormalizeCentroid(centroid);
						const uint32_t mortonCode = Morton3D(centroid.x, centroid.y, centroid.z);
						m_keysBuffer->GetLocalData()[m_trianglesLength] = mortonCode;
						m_triangleIndexBuffer->GetLocalData()[m_trianglesLength] = m_trianglesLength;
						m_triangleAABBBuffer->GetLocalData()[m_trianglesLength] = aabb;
					}
				}
			}

			m_keysBuffer->UploadCpuData();
			m_triangleIndexBuffer->UploadCpuData();
			m_triangleAABBBuffer->UploadCpuData();
			m_bvhConstructionData.SetData({.trianglesCount = m_trianglesLength});
		}

		Logger::LogFormat("Triangles length %d\n", m_trianglesLength);

		{
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
	}

	void SoftwareRaytracedDDGI::ProcessRaytracing(ID3D12GraphicsCommandList4* commandList, const uint32_t frameIndex) const
	{
		// Raytracing process
		{
			commandList->SetComputeRootSignature(m_raytracingPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_raytracingPipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "colorTexture", m_gbuffer->GetColorUAV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "normalsTexture", m_gbuffer->GetNormalsUAV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "positionTexture", m_gbuffer->GetPositionUAV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "textures", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "materials", EngineDataProvider::Get()->GetMaterialsDataView());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "sortedTriangleIndices", m_triangleIndexBuffer->GetSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "triangleAABB", m_triangleAABBBuffer->GetSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "internalNodes", m_bvhInternalNodesBuffer->GetSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "leafNodes", m_bvhLeafNodesBuffer->GetSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "bvhData", m_bvhDataBuffer->GetSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "trianglePayloadData", m_dataContainer.GetTrianglesDataView());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "meshData", m_dataContainer.GetMeshDataView());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "objectVertices", EngineDataProvider::Get()->GetMeshContainer()->GetVertexBufferSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "objectIndices", EngineDataProvider::Get()->GetMeshContainer()->GetIndexBufferSRV());
			GraphicsUtils::AttachView(commandList, m_raytracingPipeline.get(), "raytracedProbesData", m_dataContainer.GetProbesDataView(frameIndex));

			GraphicsUtils::ProcessEngineBindings(
				commandList,
				m_raytracingPipeline.get(),
				frameIndex,
				nullptr,
				nullptr);
		}
#if defined(CAMERA_TRACE)
		commandList->Dispatch((m_raytracedTextureWidth / 32) + 1, (m_raytracedTextureHeight / 32) + 1, 1);
#else
		commandList->Dispatch(g_raytracedProbesData.gridX, g_raytracedProbesData.gridY, g_raytracedProbesData.gridZ);
#endif

		m_gbuffer->BarrierColorToRead(commandList);
	}

	void SoftwareRaytracedDDGI::GenerateProbeIrradiance(ID3D12GraphicsCommandList4* commandList, uint32_t frameIndex) const
	{
		m_dataContainer.GenerateProbeIrradiance(
			commandList,
			frameIndex,
			m_shadedRenderTexture.get(),
			m_gbuffer.get(),
			m_probeIrradianceTexture.get(),
			m_probeDepthTexture.get());
	}

	void SoftwareRaytracedDDGI::DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList) const
	{
#if defined(CAMERA_TRACE)
		m_dataContainer.DebugDrawRaytracedImage(commandList, m_shadedRenderTexture->GetSRV());
#else
		m_dataContainer.DebugDrawRaytracedImage(commandList, m_probeIrradianceTexture->GetSRV());
#endif
	}

	void SoftwareRaytracedDDGI::DebugDrawAABBGizmo(ID3D12GraphicsCommandList* commandList,
	                                               const ViewProjectionMatrixData* viewProjectionMatrixData) const
	{
		auto& sm = m_debugGizmoAABBDrawerGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);


		commandList->SetGraphicsRoot32BitConstants(1, sizeof(ViewProjectionMatrixData) / 4,
		                                           viewProjectionMatrixData,
		                                           0);

		GraphicsUtils::AttachView(commandList, sm.get(), "BVHData", m_bvhDataBuffer->GetSRV());

		commandList->DrawInstanced(
			24,
			DATA_ARRAY_COUNT,
			0, 0);
	}

	void SoftwareRaytracedDDGI::DebugDrawProbes(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionMatrixData) const
	{
		m_dataContainer.DebugDrawProbes(commandList, frameIndex, viewProjectionMatrixData, m_probeIrradianceTexture->GetSRV());
	}
}
