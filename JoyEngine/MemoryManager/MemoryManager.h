#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <fstream>

#include "Common/CommandQueue.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/Texture.h"

#include "DeviceLinearAllocator.h"
#include "Common/Singleton.h"


namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager : public Singleton<MemoryManager>
	{
	public:
		MemoryManager() = default;

		void Init();

		void Start();

		void Update();

		void LoadDataToBuffer(
			std::ifstream& stream,
			uint64_t offset,
			uint64_t bufferSize,
			Buffer* gpuBuffer) const;

		void LoadDataToImage(
			std::ifstream& stream,
			uint64_t offset,
			Texture* gpuImage,
			uint32_t mipMapsCount = 1) const;

		ComPtr<ID3D12Resource> CreateResource(
			D3D12_HEAP_TYPE heapType,
			const D3D12_RESOURCE_DESC* resourceDesc,
			D3D12_RESOURCE_STATES initialResourceState,
			const D3D12_CLEAR_VALUE* clearValue = nullptr) const;

		void ChangeResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

	private:
		std::unique_ptr<CommandQueue> m_queue;
		std::array<std::unique_ptr<DeviceLinearAllocator>, 3> m_gpuHeapAllocators;
		std::array<std::unique_ptr<DeviceLinearAllocator>, 1> m_cpuHeapAllocators;
		std::unique_ptr<Buffer> m_stagingBuffer;
	};
}

#endif
