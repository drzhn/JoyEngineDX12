﻿#include "BVHConstructor.h"

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
			//shaders/raytracing/BVHTreeConstructor.hlsl
			const GUID bvhTreeConstructorShaderGuid = GUID::StringToGuid("326057ce-8c0c-40be-b912-67b63d25211e");

			m_bvhTreeConstructorPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
				{
					bvhTreeConstructorShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}

		// LOCAL RADIX SORT
		{
			//shaders/raytracing/BVHAABBMerger.hlsl
			const GUID bvhMergerShaderGuid = GUID::StringToGuid("d611cd9f-6ce0-42f5-b908-af7289b0eae5");

			m_bvhMergerPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
				{
					bvhMergerShaderGuid,
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


			GraphicsUtils::AttachViewToCompute(commandList, m_bvhTreeConstructorPipeline, "data", m_bvhConstructionData->GetView());

			GraphicsUtils::AttachViewToCompute(commandList, m_bvhTreeConstructorPipeline, "sortedMortonCodes", m_sortedMortonCodes->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhTreeConstructorPipeline, "internalNodes", m_internalNodes->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhTreeConstructorPipeline, "leafNodes", m_leafNodes->GetUAV());
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

			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "data", m_bvhConstructionData->GetView());

			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "sortedTriangleIndices", m_sortedTriangleIndices->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "triangleAABB", m_triangleAABB->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "internalNodes", m_internalNodes->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "leafNodes", m_leafNodes->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "atomicsData", m_atomics->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, m_bvhMergerPipeline, "BVHData", m_bvhData->GetUAV());
		}

		commandList->Dispatch(BLOCK_SIZE, 1, 1);
		m_dispatcher->ExecuteAndWait();
	}
}