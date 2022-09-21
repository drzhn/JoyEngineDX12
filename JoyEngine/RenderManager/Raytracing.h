#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <d3d12.h>

#include <memory>

#include "CommonEngineStructs.h"

#include "MemoryManager/MemoryManager.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Mesh.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	template <typename T>
	class DataBuffer
	{
	public:
		DataBuffer() = delete;

		DataBuffer(size_t size, T initialValue):
			DataBuffer(size)
		{
			for (int i = 0; i < size; i++)
			{
				m_dataArray[i] = initialValue;
			}

			UploadCpuData();
		}

		explicit DataBuffer(size_t size):
			m_size(size)
		{
			m_dataArray = static_cast<T*>(malloc(size * sizeof(T)));

			m_gpuBuffer = std::make_unique<Buffer>(
				size * sizeof(T),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_DEFAULT,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
		T* m_dataArray;
		const size_t m_size;
	};

	class Raytracing
	{
	public:
		Raytracing();
	private:
		uint32_t m_trianglesLength;
		std::unique_ptr<DataBuffer<uint32_t>> m_keysBuffer;
		std::unique_ptr<DataBuffer<uint32_t>> m_triangleIndexBuffer;
		std::unique_ptr<DataBuffer<Triangle>> m_triangleDataBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_triangleAABBBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_bvhDataBuffer;
		std::unique_ptr<DataBuffer<LeafNode>> m_bvhLeafNodesBuffer;
		std::unique_ptr<DataBuffer<InternalNode>> m_bvhInternalNodesBuffer;

		ResourceHandle<Mesh> m_mesh;
	};
}
#endif // RAYTRACING_H
