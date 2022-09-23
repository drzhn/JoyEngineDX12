#include "BufferSorter.h"

#include "CommonEngineStructs.h"

namespace JoyEngine
{
	BufferSorter::BufferSorter(int dataLength, DataBuffer<uint32_t>* keys, DataBuffer<uint32_t>* values):
		m_dataLength(dataLength),
		m_keys(keys),
		m_values(values)
	{
		const GUID localRadixSortShaderGuid = GUID::StringToGuid("fb423066-e885-4ea4-93d5-01b69037a9aa");
		const GUID localRaidxSortPipelineGuid = GUID::Random();

		m_localRaidxSortPipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
			localRaidxSortPipelineGuid,
			{
				localRadixSortShaderGuid,
				D3D_SHADER_MODEL_6_5
			});

		m_sortedBlocksKeysData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_sortedBlocksValuesData = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT);
		m_offsetsData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesData = std::make_unique<DataBuffer<uint32_t>>(BUCKET_SIZE * BLOCK_SIZE);
		m_sizesPrefixSumData = std::make_unique<DataBuffer<uint32_t>>(BLOCK_SIZE / (THREADS_PER_BLOCK / BUCKET_SIZE));
	}
}
