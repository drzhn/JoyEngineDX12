#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"

namespace JoyEngine
{
	template <typename T>
	class DataBuffer : public UAVGpuBuffer
	{
	public:
		DataBuffer() = delete;

		DataBuffer(uint32_t numElements, T initialValue) :
			DataBuffer(numElements)
		{
			for (uint32_t i = 0; i < numElements; i++)
			{
				m_dataArray[i] = initialValue;
			}

			UploadCpuData();
		}

		explicit DataBuffer(uint32_t numElements) :
			UAVGpuBuffer(numElements, sizeof(T))
		{
			m_dataArray = static_cast<T*>(malloc(GetSize()));
		}

		~DataBuffer()
		{
			free(m_dataArray);
		}

		void UploadCpuData() const
		{
			MemoryManager::Get()->LoadDataToBuffer(
				m_dataArray,
				GetSize(),
				m_gpuBuffer.get(), 0);
		}

		void ReadbackGpuData() const
		{
			MemoryManager::Get()->ReadbackDataFromBuffer(
				m_dataArray,
				GetSize(),
				m_gpuBuffer.get());
		}

		T* GetLocalData() { return m_dataArray; }

		void ReadbackAndWriteToMemory(void* ptr) const
		{
			ReadbackGpuData();
			memcpy(ptr, m_dataArray, GetSize());
		}

	private:
		T* m_dataArray;
	};
}
#endif // DATA_BUFFER_H
