#ifndef BUFFER_SORTER_H
#define BUFFER_SORTER_H

#include "CommonEngineStructs.h"
#include "RenderManager/Raytracing/DataBuffer.h"
#include "ResourceManager/Buffers/ConstantCpuBuffer.h"
#include "ResourceManager/Pipelines/ComputePipeline.h"

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

		std::unique_ptr<ComputePipeline> m_localRaidxSortPipeline;
		std::unique_ptr<ComputePipeline> m_preScanPipeline;
		std::unique_ptr<ComputePipeline> m_blockSumSortPipeline;
		std::unique_ptr<ComputePipeline> m_globalScanPipeline;
		std::unique_ptr<ComputePipeline> m_globalRadixSortPipeline;

		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksKeysData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksValuesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_offsetsData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesPrefixSumData;

		ConstantCpuBuffer<BufferSorterData> m_data;

		ComputeDispatcher* m_dispatcher = nullptr;
	};
}
#endif // BUFFER_SORTER_H
