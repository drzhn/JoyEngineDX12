#include "BVHConstructor.h"

#include "DescriptorManager/DescriptorManager.h"
#include "RenderManager/ComputeDispatcher.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	BVHConstructor::BVHConstructor(
		DataBuffer<uint32_t>* sortedMortonCodes,
		DataBuffer<uint32_t>* sortedTriangleIndices,
		DataBuffer<AABB>* triangleAABB,
		DataBuffer<InternalNode>* internalNodes,
		DataBuffer<LeafNode>* leafNodes,
		DataBuffer<AABB>* bvhData,
		ComputeDispatcher* dispatcher,
		ConstantCpuBuffer<BVHConstructorData>* bvhConstructionData
	) :

		m_sortedMortonCodes(sortedMortonCodes),
		m_sortedTriangleIndices(sortedTriangleIndices),
		m_triangleAABB(triangleAABB),
		m_internalNodes(internalNodes),
		m_leafNodes(leafNodes),
		m_bvhData(bvhData),
		m_bvhConstructionData(bvhConstructionData),
		m_dispatcher(dispatcher)

	{
		m_atomics = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, 0);

		// TREE CONSTRUCTOR
		{
			m_bvhTreeConstructorPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
				{
					"shaders/ddgi/sw_raytracing/BVHTreeConstructor.hlsl",
					D3D_SHADER_MODEL_6_5
				});
		}

		// LOCAL RADIX SORT
		{
			m_bvhMergerPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
				{
					"shaders/ddgi/sw_raytracing/BVHAABBMerger.hlsl",
					D3D_SHADER_MODEL_6_5
				});
		}
	}

	void BVHConstructor::ConstructTree()
	{
		TIME_PERF("Scene BVH tree construction");

		const auto commandList = m_dispatcher->GetCommandList();

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		// Tree construction
		{
			commandList->SetComputeRootSignature(m_bvhTreeConstructorPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_bvhTreeConstructorPipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachView(commandList, m_bvhTreeConstructorPipeline.get(), "data", m_bvhConstructionData->GetView());

			GraphicsUtils::AttachView(commandList, m_bvhTreeConstructorPipeline.get(), "sortedMortonCodes", m_sortedMortonCodes->GetSRV());
			GraphicsUtils::AttachView(commandList, m_bvhTreeConstructorPipeline.get(), "internalNodes", m_internalNodes->GetUAV());
			GraphicsUtils::AttachView(commandList, m_bvhTreeConstructorPipeline.get(), "leafNodes", m_leafNodes->GetUAV());
		}

		commandList->Dispatch(BLOCK_SIZE, 1, 1);
		m_dispatcher->ExecuteAndWait();
	}

	void BVHConstructor::ConstructBVH() const
	{
		TIME_PERF("Scene BVH merge");

		const auto commandList = m_dispatcher->GetCommandList();

		ID3D12DescriptorHeap* heaps[2]
		{
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
			DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
		};
		commandList->SetDescriptorHeaps(2, heaps);

		{
			// Bvh merge
			commandList->SetComputeRootSignature(m_bvhMergerPipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_bvhMergerPipeline->GetPipelineObject().Get());

			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "data", m_bvhConstructionData->GetView());

			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "sortedTriangleIndices", m_sortedTriangleIndices->GetSRV());
			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "triangleAABB", m_triangleAABB->GetSRV());
			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "internalNodes", m_internalNodes->GetSRV());
			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "leafNodes", m_leafNodes->GetSRV());
			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "atomicsData", m_atomics->GetUAV());
			GraphicsUtils::AttachView(commandList, m_bvhMergerPipeline.get(), "BVHData", m_bvhData->GetUAV());
		}

		commandList->Dispatch(BLOCK_SIZE, 1, 1);
		m_dispatcher->ExecuteAndWait();
	}
}
