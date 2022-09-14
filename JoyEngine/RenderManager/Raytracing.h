#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <d3d12.h>

#include <memory>

#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/Buffer.h"

namespace JoyEngine
{
	template <typename T, size_t Size>
	class DataBuffer
	{
	public:
		DataBuffer(T initialValue):
			DataBuffer()
		{
			for (int i = 0; i < Size; i++)
			{
				m_dataArray[i] = initialValue;
			}

			UploadCpuData();
		}

		DataBuffer()
		{
			m_gpuBuffer = std::make_unique<Buffer>(
				Size * sizeof(T),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}

		void UploadCpuData()
		{
			MemoryManager::Get()->LoadDataToBuffer(
				m_dataArray.data(), 
				Size * sizeof(T), 
				m_gpuBuffer.get());
		}

		void ReadbackGpuData()
		{
			MemoryManager::Get()->ReadbackDataFromBuffer(
				m_dataArray.data(), 
				Size * sizeof(T),
				m_gpuBuffer.get());
		}

		std::array<T, Size>& GetLocalData() { return m_dataArray; }

	private:
		std::unique_ptr<Buffer> m_gpuBuffer;
		std::array<T, Size> m_dataArray;
	};

	class Raytracing
	{
	public:
		Raytracing();
	private:
		std::unique_ptr<DataBuffer<uint32_t, 100>> m_testBuffer;
	};
}
#endif // RAYTRACING_H
