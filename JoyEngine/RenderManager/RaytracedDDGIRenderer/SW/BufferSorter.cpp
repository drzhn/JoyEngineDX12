#include "BufferSorter.h"

#include "CommonEngineStructs.h"
#include "Common/HashDefs.h"
#include "DescriptorManager/DescriptorManager.h"
#include "RenderManager/ComputeDispatcher.h"
#include "Utils/GraphicsUtils.h"
#include "Utils/TimeCounter.h"

namespace JoyEngine
{
	BufferSorter::BufferSorter(int dataLength, DataBuffer<uint32_t>* keys, DataBuffer<uint32_t>* values, ComputeDispatcher* dispatcher):
		m_dataLength(dataLength),
		m_keys(keys),
		m_values(values),
		m_dispatcher(dispatcher)
	{
		{
			// LOCAL RADIX SORT
			{
				m_localRaidxSortPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/ddgi/sw_raytracing/LocalRadixSort.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
		}
		{
			// SCAN 
			{
				m_preScanPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/ddgi/sw_raytracing/PreScan.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
			{
				m_blockSumSortPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/ddgi/sw_raytracing/BlockSum.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
			{
				m_globalScanPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/ddgi/sw_raytracing/GlobalScan.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
		}

		{
			// GLOBAL RADIX SORT
			{
				m_globalRadixSortPipeline = std::make_unique<ComputePipeline>(ComputePipelineArgs
					{
						"shaders/ddgi/sw_raytracing/GlobalRadixSort.hlsl",
						D3D_SHADER_MODEL_6_5
					});
			}
		}

		m_sortedBlocksKeysData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_sortedBlocksValuesData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_offsetsData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesPrefixSumData = std::make_unique<DataBuffer<uint32_t>>(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE));
	}

	void BufferSorter::Sort()
	{
		TIME_PERF("ComputeBuffer sorter")

		for (uint32_t bitOffset = 0; bitOffset < sizeof(uint32_t) * 8; bitOffset += RADIX)
		{
			m_data.SetData({.bitOffset = bitOffset});
			const auto commandList = m_dispatcher->GetCommandList();

			ID3D12DescriptorHeap* heaps[2]
			{
				DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
				DescriptorManager::Get()->GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
			};
			commandList->SetDescriptorHeaps(2, heaps);

			// Local radix sort
			{
				commandList->SetComputeRootSignature(m_localRaidxSortPipeline->GetRootSignature().Get());
				commandList->SetPipelineState(m_localRaidxSortPipeline->GetPipelineObject().Get());


				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "data", m_data.GetView());

				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "keysData", m_keys->GetSRV());
				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "valuesData", m_values->GetSRV());
				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "sortedBlocksKeysData", m_sortedBlocksKeysData->GetUAV());
				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "sortedBlocksValuesData", m_sortedBlocksValuesData->GetUAV());
				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "offsetsData", m_offsetsData->GetUAV());
				GraphicsUtils::AttachView(commandList, m_localRaidxSortPipeline.get(), "sizesData", m_sizesData->GetUAV());

				commandList->Dispatch(BLOCK_SIZE, 1, 1);
			}

			GraphicsUtils::UAVBarrier(commandList, m_keys->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_values->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_sortedBlocksKeysData->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_sortedBlocksValuesData->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_offsetsData->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_sizesData->GetBuffer()->GetBufferResource().Get());

			// Pre scan 
			{
				commandList->SetComputeRootSignature(m_preScanPipeline->GetRootSignature().Get());
				commandList->SetPipelineState(m_preScanPipeline->GetPipelineObject().Get());


				GraphicsUtils::AttachView(commandList, m_preScanPipeline.get(), "data", m_sizesData->GetUAV());
				GraphicsUtils::AttachView(commandList, m_preScanPipeline.get(), "blockSumsData", m_sizesPrefixSumData->GetUAV());

				commandList->Dispatch(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE), 1, 1);
			}

			GraphicsUtils::UAVBarrier(commandList, m_sizesData->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_sizesPrefixSumData->GetBuffer()->GetBufferResource().Get());

			// Block sum
			{
				commandList->SetComputeRootSignature(m_blockSumSortPipeline->GetRootSignature().Get());
				commandList->SetPipelineState(m_blockSumSortPipeline->GetPipelineObject().Get());


				GraphicsUtils::AttachView(commandList, m_blockSumSortPipeline.get(), "blockSumsData", m_sizesPrefixSumData->GetUAV());

				commandList->Dispatch(1, 1, 1);
			}

			GraphicsUtils::UAVBarrier(commandList, m_sizesPrefixSumData->GetBuffer()->GetBufferResource().Get());

			// Global scan
			{
				commandList->SetComputeRootSignature(m_globalScanPipeline->GetRootSignature().Get());
				commandList->SetPipelineState(m_globalScanPipeline->GetPipelineObject().Get());


				GraphicsUtils::AttachView(commandList, m_globalScanPipeline.get(), "data", m_sizesData->GetUAV());
				GraphicsUtils::AttachView(commandList, m_globalScanPipeline.get(), "blockSumsData", m_sizesPrefixSumData->GetUAV());

				commandList->Dispatch(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE), 1, 1);
			}

			GraphicsUtils::UAVBarrier(commandList, m_sizesData->GetBuffer()->GetBufferResource().Get());
			GraphicsUtils::UAVBarrier(commandList, m_sizesPrefixSumData->GetBuffer()->GetBufferResource().Get());

			// Global radix sort
			{
				commandList->SetComputeRootSignature(m_globalRadixSortPipeline->GetRootSignature().Get());
				commandList->SetPipelineState(m_globalRadixSortPipeline->GetPipelineObject().Get());


				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "data", m_data.GetView());

				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "sortedBlocksKeysData", m_sortedBlocksKeysData->GetSRV());
				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "sortedBlocksValuesData", m_sortedBlocksValuesData->GetSRV());
				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "offsetsData", m_offsetsData->GetSRV());
				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "sizesData", m_sizesData->GetSRV());
				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "sortedKeysData", m_keys->GetUAV());
				GraphicsUtils::AttachView(commandList, m_globalRadixSortPipeline.get(), "sortedValuesData", m_values->GetUAV());

				commandList->Dispatch(BLOCK_SIZE, 1, 1);
			}
			m_dispatcher->ExecuteAndWait();
		}

#ifdef _DEBUG
		m_keys->ReadbackGpuData();
		//Logger::Log("\n\n");
		//Logger::LogUintArray(m_keys->GetLocalData(), m_keys->GetSize(), 1030);

		for (int i = 0; i < BLOCK_SIZE * THREADS_PER_BLOCK - 1; i++)
		{
			uint32_t first = m_keys->GetLocalData()[i];
			uint32_t second = m_keys->GetLocalData()[i + 1];
			ASSERT(first <= second);
		}
#endif
	}
}
