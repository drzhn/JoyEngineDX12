#include "Raytracing.h"

#include "Common/HashDefs.h"
#include "ResourceManager/ResourceManager.h"
#include "Components/MeshRenderer.h"
#include "DescriptorManager/DescriptorManager.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/Material.h"
#include "SceneManager/Transform.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/Log.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	AABB Whole = {
		.min = glm::vec3(-40.0f, -3.0f, -25.0f),
		.max = glm::vec3(40.0f, 30.0f, 25.0f),
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
		ret.x -= Whole.min.x;
		ret.y -= Whole.min.y;
		ret.z -= Whole.min.z;
		ret.x /= (Whole.max.x - Whole.min.x);
		ret.y /= (Whole.max.y - Whole.min.y);
		ret.z /= (Whole.max.z - Whole.min.z);
		return ret;
	}

	Raytracing::Raytracing(
		std::set<SharedMaterial*>& sceneSharedMaterials,
		DXGI_FORMAT mainColorFormat,
		DXGI_FORMAT swapchainFormat,
		DXGI_FORMAT gBufferPositionsFormat,
		DXGI_FORMAT gBufferNormalsFormat,
		uint32_t width, uint32_t height) :
		m_mainColorFormat(mainColorFormat),
		m_gBufferPositionsFormat(gBufferPositionsFormat),
		m_gBufferNormalsFormat(gBufferNormalsFormat),
		m_swapchainFormat(swapchainFormat),
		m_width(width),
		m_height(height),
		m_sceneSharedMaterials(sceneSharedMaterials)
	{
		static_assert(sizeof(Triangle) == 128);
		static_assert(sizeof(AABB) == 32);

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
			m_colorTexture = std::make_unique<UAVTexture>(
				m_width,
				m_height,
				m_mainColorFormat,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT
			);

			m_positionsTexture = std::make_unique<UAVTexture>(
				m_width,
				m_height,
				m_mainColorFormat,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT
			);

			m_normalsTexture = std::make_unique<UAVTexture>(
				m_width,
				m_height,
				m_mainColorFormat,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT
			);


			//shaders/raytracing/Raytracing.hlsl
			const GUID raytracingShaderGuid = GUID::StringToGuid("b24e90ac-fcfa-4754-b0e5-8553b19e27ca");
			const GUID raytracingPipelineGuid = GUID::Random();

			m_raytracingPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				raytracingPipelineGuid,
				{
					raytracingShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}

		// Draw raytraced texture 
		{
			const GUID debugImageComposerShaderGuid = GUID::StringToGuid("cc8de13c-0510-4842-99f5-de2327aa95d4"); // shaders/raytracing/debugImageCompose.hlsl
			const GUID debugImageComposerSharedMaterialGuid = GUID::Random();

			const D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {
				true,
				FALSE,
				D3D12_BLEND_SRC_ALPHA,
				D3D12_BLEND_INV_SRC_ALPHA,
				D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE,
				D3D12_BLEND_ZERO,
				D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL
			};

			D3D12_BLEND_DESC blend = {
				.AlphaToCoverageEnable = false,
				.IndependentBlendEnable = false,
			};
			blend.RenderTarget[0] = blendDesc;

			m_debugRaytracingTextureDrawGraphicsPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
				debugImageComposerSharedMaterialGuid,
				{
					debugImageComposerShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_NEVER,
					CD3DX12_BLEND_DESC(blend),
					{
						mainColorFormat
					},
					DXGI_FORMAT_UNKNOWN,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Gizmo AABB draw 
		{
			const GUID gizmoAABBDrawerShaderGuid = GUID::StringToGuid("a231c467-dc15-4753-a3db-8888efc73c1a"); // shaders/raytracing/gizmoAABBDrawer.hlsl
			const GUID gizmoAABBDrawerSharedMaterialGuid = GUID::Random();

			m_gizmoAABBDrawerGraphicsPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
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
	}

	void Raytracing::UploadSceneData()
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
						.objectIndex = *mr->GetTransform()->GetIndex(),
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

	void Raytracing::PrepareBVH()
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

	void Raytracing::ProcessRaytracing(ID3D12GraphicsCommandList* commandList, ResourceView* engineDataResourceView)
	{
		// Raytracing process
		{
			commandList->SetComputeRootSignature(m_raytracingPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_raytracingPipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "engineData", engineDataResourceView);

			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "colorTexture", m_colorTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "positionsTexture", m_positionsTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "normalsTexture", m_normalsTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "textures", DescriptorManager::Get()->GetSRVHeapStartDescriptorHandle());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "materials", EngineMaterialProvider::Get()->GetMaterialsDataView());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "sortedTriangleIndices", m_triangleIndexBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleAABB", m_triangleAABBBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "internalNodes", m_bvhInternalNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "leafNodes", m_bvhLeafNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "bvhData", m_bvhDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleData", m_triangleDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "linearClampSampler", EngineSamplersProvider::GetLinearWrapSampler());

			GraphicsUtils::UAVBarrier(commandList, m_colorTexture->GetImage().Get());
		}

		commandList->Dispatch((m_width / 32) + 1, (m_height / 32) + 1, 1);
	}

	void Raytracing::DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList)
	{
		auto sm = m_debugRaytracingTextureDrawGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		GraphicsUtils::AttachViewToGraphics(commandList, sm, "raytracingTexture", m_colorTexture->GetSRV());
		commandList->DrawInstanced(3, 1, 0, 0);
	}

	void Raytracing::DrawGizmo(ID3D12GraphicsCommandList* commandList, const ViewProjectionMatrixData* viewProjectionMatrixData) const
	{
		auto sm = m_gizmoAABBDrawerGraphicsPipeline;

		commandList->SetPipelineState(sm->GetPipelineObject().Get());
		commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);


		commandList->SetGraphicsRoot32BitConstants(1,
		                                           sizeof(ViewProjectionMatrixData) / 4,
		                                           viewProjectionMatrixData,
		                                           0);

		GraphicsUtils::AttachViewToGraphics(commandList, sm, "BVHData", m_bvhDataBuffer->GetSRV());

		commandList->DrawInstanced(
			24,
			DATA_ARRAY_COUNT,
			0, 0);
	}
}
