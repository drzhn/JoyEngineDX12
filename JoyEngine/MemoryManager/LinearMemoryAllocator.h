#ifndef DEVICE_LINEAR_ALLOCATOR_H
#define DEVICE_LINEAR_ALLOCATOR_H

#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

#include "Common/Allocators/LinearAllocator.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class LinearAllocator;

	enum DeviceAllocatorType
	{
		DeviceAllocatorTypeGpuBuffer = 0,
		DeviceAllocatorTypeTextures = 1,
		DeviceAllocatorTypeRtDsTextures = 2,
		DeviceAllocatorTypeCpuUploadBuffer = 3,
		DeviceAllocatorTypeCpuReadbackBuffer = 4,
	};

	class LinearMemoryAllocator
	{
	public:
		LinearMemoryAllocator() = default;

		LinearMemoryAllocator(D3D12_HEAP_TYPE heapType, DeviceAllocatorType type, uint64_t size, ID3D12Device* device);
		uint64_t Allocate(uint64_t size);
		[[nodiscard]] ID3D12Heap* GetHeap() const;

		[[nodiscard]] uint64_t GetAlignedBytesAllocated() const noexcept { return m_allocator.GetCurrentOffset(); }
		[[nodiscard]] uint64_t GetUnalignedBytesAllocated() const noexcept { return m_unalignedBytesAllocated; }
		[[nodiscard]] uint64_t GetSize() const noexcept { return m_allocator.GetSize(); }

	private:
		ID3D12Device* m_device;
		ComPtr<ID3D12Heap> m_heap;
		LinearAllocator m_allocator;

		uint64_t m_unalignedBytesAllocated = 0;
	};
}
#endif // DEVICE_LINEAR_ALLOCATOR_H
