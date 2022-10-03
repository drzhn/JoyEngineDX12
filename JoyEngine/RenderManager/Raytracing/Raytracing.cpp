#include "Raytracing.h"

#include "Common/HashDefs.h"
#include "ResourceManager/ResourceManager.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/Log.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	AABB Whole = {
		.min = glm::vec3(-15.5f, -15.5f, -15.5f),
		.max = glm::vec3(15.5f, 15.5f, 15.5f),
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

	Raytracing::Raytracing(DXGI_FORMAT mainColorFormat, DXGI_FORMAT swapchainFormat, uint32_t width, uint32_t height):
		m_mainColorFormat(mainColorFormat),
		m_swapchainFormat(swapchainFormat),
		m_width(width),
		m_height(height)
	{
		static_assert(sizeof(Triangle) == 128);
		static_assert(sizeof(AABB) == 32);

		const GUID meshGuid = GUID::StringToGuid("f9685ae2-2293-4662-83c1-69a37d1499fe");
		m_mesh = ResourceManager::Get()->LoadResource<Mesh>(meshGuid);

		const GUID textureGuid = GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0");
		m_texture = ResourceManager::Get()->LoadResource<Texture>(textureGuid);

		m_keysBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleIndexBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleDataBuffer = std::make_unique<DataBuffer<Triangle>>(DATA_ARRAY_COUNT);
		m_triangleAABBBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);

		m_bvhDataBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);
		m_bvhLeafNodesBuffer = std::make_unique<DataBuffer<LeafNode>>(DATA_ARRAY_COUNT);
		m_bvhInternalNodesBuffer = std::make_unique<DataBuffer<InternalNode>>(DATA_ARRAY_COUNT);

		m_trianglesLength = m_mesh->GetIndexCount() / 3;

		Vertex* vertices = m_mesh->GetVertices();
		uint32_t* indices = m_mesh->GetIndices();

		for (uint32_t i = 0; i < m_trianglesLength; i++)
		{
			glm::vec3 a = vertices[indices[i * 3 + 0]].pos;
			glm::vec3 b = vertices[indices[i * 3 + 1]].pos;
			glm::vec3 c = vertices[indices[i * 3 + 2]].pos;

			glm::vec3 centroid;
			AABB aabb = {};

			GetCentroidAndAABB(a, b, c, &centroid, &aabb);
			centroid = NormalizeCentroid(centroid);
			uint32_t mortonCode = Morton3D(centroid.x, centroid.y, centroid.z);
			m_keysBuffer->GetLocalData()[i] = mortonCode;
			m_triangleIndexBuffer->GetLocalData()[i] = i;
			m_triangleDataBuffer->GetLocalData()[i] = Triangle
			{
				.a = a,
				.b = b,
				.c = c,
				.a_uv = vertices[indices[i * 3 + 0]].texCoord,
				.b_uv = vertices[indices[i * 3 + 1]].texCoord,
				.c_uv = vertices[indices[i * 3 + 2]].texCoord,
				.a_normal = vertices[indices[i * 3 + 0]].normal,
				.b_normal = vertices[indices[i * 3 + 1]].normal,
				.c_normal = vertices[indices[i * 3 + 2]].normal,
			};
			m_triangleAABBBuffer->GetLocalData()[i] = aabb;
		}

		m_keysBuffer->UploadCpuData();
		m_triangleIndexBuffer->UploadCpuData();
		m_triangleDataBuffer->UploadCpuData();
		m_triangleAABBBuffer->UploadCpuData();

		m_dispatcher = std::make_unique<ComputeDispatcher>();

		m_bufferSorter = std::make_unique<BufferSorter>(
			m_trianglesLength,
			m_keysBuffer.get(),
			m_triangleIndexBuffer.get(),
			m_dispatcher.get());

		m_bvhConstructionData.SetData({.trianglesCount = m_trianglesLength});

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
			m_raytracedTexture = std::make_unique<UAVTexture>(
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

		m_bvhDataBuffer->ReadbackGpuData();
		m_bvhLeafNodesBuffer->ReadbackGpuData();
		m_bvhInternalNodesBuffer->ReadbackGpuData();

		//for (int i = 0; i < 14; i++)
		//{
		//	AABB data = m_bvhDataBuffer->GetLocalData()[i];
		//	Logger::LogFormat("%.3f %.3f %.3f %.3f %.3f %.3f \n",
		//	                  data.min.x,
		//	                  data.min.y,
		//	                  data.min.z,
		//	                  data.max.x,
		//	                  data.max.y,
		//	                  data.max.z);
		//}

		//for (int i = 0; i < 14; i++)
		//{
		//	LeafNode data = m_bvhLeafNodesBuffer->GetLocalData()[i];
		//	Logger::LogFormat("index %d, parent %d\n", data.index, data.parent);
		//}

		//for (int i = 0; i < 14; i++)
		//{
		//	InternalNode data = m_bvhInternalNodesBuffer->GetLocalData()[i];
		//	Logger::LogFormat("index %d, left %d, right %d, parent %d\n", data.index, data.leftNode, data.rightNode, data.parent);
		//}
	}

	void Raytracing::ProcessRaytracing(ID3D12GraphicsCommandList* commandList, ResourceView* engineDataResourceView)
	{
		// Raytracing process
		{
			commandList->SetComputeRootSignature(m_raytracingPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_raytracingPipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "engineData", engineDataResourceView);

			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "_outputTexture", m_raytracedTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "_meshTexture", m_texture->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "sortedTriangleIndices", m_triangleIndexBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleAABB", m_triangleAABBBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "internalNodes", m_bvhInternalNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "leafNodes", m_bvhLeafNodesBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "bvhData", m_bvhDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "triangleData", m_triangleDataBuffer->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_raytracingPipeline, "linearClampSampler", EngineSamplersProvider::GetLinearClampSampler());
		}

		commandList->Dispatch((m_width / 32) + 1, (m_height / 32) + 1, 1);
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
