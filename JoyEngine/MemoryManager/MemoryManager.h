#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <fstream>
#include <map>
#include <unordered_map>

#include "Common/CommandQueue.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"

#include "DeviceLinearAllocator.h"


namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager
	{
	public:
		MemoryManager() = default;

		void Init();

		void Update();

		void LoadDataToBuffer(
			std::ifstream& stream,
			uint64_t offset,
			uint64_t bufferSize,
			Buffer* gpuBuffer) const;

		void LoadDataToImage(
			std::ifstream& stream,
			uint64_t offset,
			uint32_t RowPitch,
			uint32_t SlicePitch,
			uint32_t width,
			uint32_t height,
			Texture* gpuImage,
			uint32_t mipMapsCount = 1) const;

		//ComPtr<ID3D12Resource> CreateResource(
		//	D3D12_HEAP_TYPE heapType,
		//	const D3D12_RESOURCE_DESC* resourceDesc,
		//	D3D12_RESOURCE_STATES initialResourceState,
		//	const D3D12_CLEAR_VALUE* clearValue = nullptr);

		void ChangeResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

	private:
		std::unique_ptr<CommandQueue> m_queue;
		std::unordered_map<D3D12_HEAP_FLAGS, std::unique_ptr<DeviceLinearAllocator>> m_gpuHeapAllocators;
		std::unordered_map<D3D12_HEAP_FLAGS, std::unique_ptr<DeviceLinearAllocator>> m_cpuHeapAllocators;

	};
}

#endif
