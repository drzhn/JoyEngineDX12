#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	template <typename T>
	class DataBuffer
	{
	public:
		DataBuffer() = delete;

		DataBuffer(size_t size, T initialValue) :
			DataBuffer(size)
		{
			for (int i = 0; i < size; i++)
			{
				m_dataArray[i] = initialValue;
			}

			UploadCpuData();
		}

		explicit DataBuffer(size_t size) :
			m_size(size)
		{
			m_dataArray = static_cast<T*>(malloc(size * sizeof(T)));

			m_gpuBuffer = std::make_unique<Buffer>(
				size * sizeof(T),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
				.Buffer = {
					.FirstElement = 0,
					.NumElements = static_cast<uint32_t>(size),
					.StructureByteStride = sizeof(T),
					.CounterOffsetInBytes = 0,
					.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
				}
			};

			m_bufferView = std::make_unique<ResourceView>(desc, m_gpuBuffer->GetBuffer().Get());
		}

		~DataBuffer()
		{
			free(m_dataArray);
		}

		void UploadCpuData() const
		{
			MemoryManager::Get()->LoadDataToBuffer(
				m_dataArray,
				m_size * sizeof(T),
				m_gpuBuffer.get());
		}

		void ReadbackGpuData() const
		{
			MemoryManager::Get()->ReadbackDataFromBuffer(
				m_dataArray,
				m_size * sizeof(T),
				m_gpuBuffer.get());
		}

		T* GetLocalData() { return m_dataArray; }

	private:
		std::unique_ptr<Buffer> m_gpuBuffer;
		std::unique_ptr<ResourceView> m_bufferView;
		T* m_dataArray;
		const size_t m_size;
	};
}
#endif // DATA_BUFFER_H
