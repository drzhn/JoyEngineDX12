#ifndef BUFFER_SORTER_H
#define BUFFER_SORTER_H

#include "CommonEngineStructs.h"
#include "DataBuffer.h"
#include "ResourceManager/ConstantBuffer.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	class ComputeDispatcher;

	class BufferSorter
	{
	public:
		BufferSorter() = delete;
		explicit BufferSorter(int dataLength, DataBuffer<uint32_t>* keys, DataBuffer<uint32_t>* values, ComputeDispatcher* dispatcher);
		void Sort();
	private:
		uint32_t m_dataLength;
		DataBuffer<uint32_t>* m_keys;
		DataBuffer<uint32_t>* m_values;

		ResourceHandle<ComputePipeline> m_localRaidxSortPipeline;
		ResourceHandle<ComputePipeline> m_preScanPipeline;
		ResourceHandle<ComputePipeline> m_blockSumSortPipeline;
		ResourceHandle<ComputePipeline> m_globalScanPipeline;
		ResourceHandle<ComputePipeline> m_globalRadixSortPipeline;

		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksKeysData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksValuesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_offsetsData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesPrefixSumData;

		ConstantBuffer<BufferSorterData> m_data;

		ComputeDispatcher* m_dispatcher = nullptr;
	};
}
#endif // BUFFER_SORTER_H
