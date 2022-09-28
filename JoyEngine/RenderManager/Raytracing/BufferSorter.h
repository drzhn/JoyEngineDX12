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

		ConstantBuffer<RaytracingData> m_raytracingData;

		ComputeDispatcher* m_dispatcher = nullptr;

//#ifdef _DEBUG
		std::array<uint32_t, DATA_ARRAY_COUNT> m_unsortedKeysLocalData = std::array<uint32_t, DATA_ARRAY_COUNT>();
		std::array<uint32_t, DATA_ARRAY_COUNT> m_sortedKeysLocalData = std::array<uint32_t, DATA_ARRAY_COUNT>();
		std::array<uint32_t, BUCKET_SIZE * BLOCK_SIZE> m_sizesLocalDataBeforeScan = std::array<uint32_t, BUCKET_SIZE * BLOCK_SIZE>();

		std::array<int, BUCKET_SIZE> _debugDataArray = std::array<int, BUCKET_SIZE>();
		//#endif
	};
}
#endif // BUFFER_SORTER_H
