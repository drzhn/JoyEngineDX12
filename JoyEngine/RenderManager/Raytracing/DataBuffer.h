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
			m_dataArray = static_cast<T*>(malloc(m_size * sizeof(T)));

			m_gpuBuffer = std::make_unique<Buffer>(
				m_size * sizeof(T),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
				.Buffer = {
					.FirstElement = 0,
					.NumElements = static_cast<uint32_t>(m_size),
					.StructureByteStride = sizeof(T),
					.CounterOffsetInBytes = 0,
					.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
				}
			};
			m_bufferViewUAV = std::make_unique<ResourceView>(unorderedAccessViewDesc, m_gpuBuffer->GetBuffer().Get());


			D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
				.Format = DXGI_FORMAT_UNKNOWN,
				.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Buffer = {
					0,
					static_cast<uint32_t>(m_size),
					sizeof(T),
					D3D12_BUFFER_SRV_FLAG_NONE
				}
			};
			m_bufferViewSRV = std::make_unique<ResourceView>(shaderResourceViewDesc, m_gpuBuffer->GetBuffer().Get());
		}

		~DataBuffer()
		{
			free(m_dataArray);
		}

		[[nodiscard]] ResourceView* GetUAV() const
		{
			return m_bufferViewUAV.get();
		}

		[[nodiscard]] ResourceView* GetSRV() const
		{
			return m_bufferViewSRV.get();
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

		void ReadbackAndWriteToMemory(void* ptr) const
		{
			ReadbackGpuData();
			memcpy(ptr, m_dataArray, m_size * sizeof(T));
		}

		[[nodiscard]] size_t GetSize() const noexcept { return m_size; }
		Buffer* GetBuffer() const noexcept { return m_gpuBuffer.get(); }

	private:
		std::unique_ptr<Buffer> m_gpuBuffer;
		std::unique_ptr<ResourceView> m_bufferViewUAV;
		std::unique_ptr<ResourceView> m_bufferViewSRV;
		T* m_dataArray;
		const size_t m_size;
	};
}
#endif // DATA_BUFFER_H
