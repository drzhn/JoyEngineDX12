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
		{ //shaders/raytracing/LocalRadixSort.hlsl
			const GUID localRadixSortShaderGuid = GUID::StringToGuid("fb423066-e885-4ea4-93d5-01b69037a9aa"); 
			const GUID localRaidxSortPipelineGuid = GUID::Random();

			m_localRaidxSortPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				localRaidxSortPipelineGuid,
				{
					localRadixSortShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}
		{ //shaders/raytracing/PreScan.hlsl
			const GUID preScanShaderGuid = GUID::StringToGuid("8190cec8-aa5a-420e-8de2-34c6f838476c"); 
			const GUID preScanPipelineGuid = GUID::Random();

			m_preScanPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				preScanPipelineGuid,
				{
					preScanShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}
		{ //shaders/raytracing/BlockSum.hlsl
			const GUID blockSumShaderGuid = GUID::StringToGuid("d00ec3ea-53c4-48f3-bc42-d775c67db85c"); 
			const GUID blockSumPipelineGuid = GUID::Random();

			m_blockSumSortPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				blockSumPipelineGuid,
				{
					blockSumShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}
		{ //shaders/raytracing/GlobalScan.hlsl
			const GUID globalScanShaderGuid = GUID::StringToGuid("8e67cd9b-afdf-409e-bb98-a79ad3f692c8"); 
			const GUID globalScanPipelineGuid = GUID::Random();

			m_globalScanPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				globalScanPipelineGuid,
				{
					globalScanShaderGuid,
					D3D_SHADER_MODEL_6_5
				});
		}

		m_sortedBlocksKeysData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_sortedBlocksValuesData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_offsetsData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesPrefixSumData = std::make_unique<DataBuffer<uint32_t>>(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE));


		m_keys->ReadbackAndWriteToMemory(m_unsortedKeysLocalData.data());

		Logger::LogUintArray(m_unsortedKeysLocalData.data(), m_unsortedKeysLocalData.size());

		for (int i = 0; i < BUCKET_SIZE; i++)
		{
			_debugDataArray[i] = 0;
		}
	}

	void BufferSorter::Sort()
	{
		TIME_PERF("ComputeBuffer sorter")

		for (uint32_t bitOffset = 0; bitOffset < sizeof(uint32_t) * 8; bitOffset += RADIX)
		{
			m_raytracingData.SetData({.bitOffset = bitOffset});
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


				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "raytracingData", m_raytracingData.GetView());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "keysData", m_keys->GetSRV());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "valuesData", m_values->GetSRV());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "sortedBlocksKeysData", m_sortedBlocksKeysData->GetUAV());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "sortedBlocksValuesData", m_sortedBlocksValuesData->GetUAV());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "offsetsData", m_offsetsData->GetUAV());
				GraphicsUtils::AttachViewToCompute(commandList, m_localRaidxSortPipeline, "sizesData", m_sizesData->GetUAV());

				commandList->Dispatch(BLOCK_SIZE, 1, 1);
			}
			m_dispatcher->ExecuteAndWait();
		}

		m_sortedBlocksKeysData->ReadbackGpuData();
		Logger::Log("\n\n");
		Logger::LogUintArray(m_sortedBlocksKeysData->GetLocalData(), m_sortedBlocksKeysData->GetSize());

		//for (int i = 0; i < BLOCK_SIZE; i++)
		//{
		//	for (int j = 0; j < THREADS_PER_BLOCK - 1; j++)
		//	{
		//		uint32_t first = m_sortedBlocksKeysData->GetLocalData()[i * THREADS_PER_BLOCK + j];
		//		uint32_t second = m_sortedBlocksKeysData->GetLocalData()[i * THREADS_PER_BLOCK + j + 1];
		//		ASSERT(first <= second);
		//	}
		//}
	}
}
