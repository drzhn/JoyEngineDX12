#ifndef BUFFER_SORTER_H
#define BUFFER_SORTER_H

#include "DataBuffer.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	class BufferSorter
	{
	public:
		BufferSorter() = delete;
		explicit BufferSorter(int dataLength, DataBuffer<uint32_t>* keys, DataBuffer<uint32_t>* values);
	private:
		uint32_t m_dataLength;
		DataBuffer<uint32_t>* m_keys;
		DataBuffer<uint32_t>* m_values;

		ResourceHandle<ComputePipeline> m_localRaidxSortPipeline;

		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksKeysData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sortedBlocksValuesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_offsetsData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesData;
		std::unique_ptr<DataBuffer<uint32_t>> m_sizesPrefixSumData;
	};
}
#endif // BUFFER_SORTER_H
