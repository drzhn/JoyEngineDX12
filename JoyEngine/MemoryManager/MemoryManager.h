#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <fstream>

#include "Common/CommandQueue.h"
#include "ResourceManager/Texture.h"

#include "LinearMemoryAllocator.h"
#include "Common/Singleton.h"
#include "ResourceManager/Buffers/Buffer.h"


namespace JoyEngine
{
	class JoyEngine;

	class MemoryManager : public Singleton<MemoryManager>
	{
	public:
		MemoryManager() = default;

		void Init();

		void PrintStats() const;

		void Update();

		void LoadDataToBuffer(
			std::ifstream& stream,
			uint64_t offset,
			uint64_t bufferSize,
			const Buffer* gpuBuffer) const;

		void LoadDataToBuffer(
			void* ptr,
			uint64_t bufferSize,
			const Buffer* gpuBuffer, uint64_t bufferOffset) const;

		void LoadDataToImage(
			std::ifstream& stream,
			uint64_t offset,
			Texture* gpuImage) const;

		void ReadbackDataFromBuffer(
			void* ptr,
			uint64_t bufferSize,
			const Buffer* gpuBuffer) const;

		ComPtr<ID3D12Resource> CreateResource(
			D3D12_HEAP_TYPE heapType,
			const D3D12_RESOURCE_DESC* resourceDesc,
			D3D12_RESOURCE_STATES initialResourceState,
			const D3D12_CLEAR_VALUE* clearValue = nullptr) const;

	private:
		void LoadDataToBufferInternal(uint64_t bufferSize, const Buffer* gpuBuffer, uint64_t bufferOffset) const;

		std::unique_ptr<CommandQueue> m_queue;
		std::array<std::unique_ptr<LinearMemoryAllocator>, 5> m_allocators;
		std::unique_ptr<Buffer> m_uploadStagingBuffer;
		std::unique_ptr<Buffer> m_readbackStagingBuffer;
	};
}

#endif
