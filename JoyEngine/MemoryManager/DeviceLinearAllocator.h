#ifndef DEVICE_LINEAR_ALLOCATOR_H
#define DEVICE_LINEAR_ALLOCATOR_H


#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	enum DeviceAllocatorType
	{
		DeviceAllocatorTypeBuffer = 0,
		DeviceAllocatorTypeTextures = 1,
		DeviceAllocatorTypeRtDsTextures = 2
	};

	class DeviceLinearAllocator
	{
	public:
		DeviceLinearAllocator() = default;

		DeviceLinearAllocator(D3D12_HEAP_TYPE heapType, DeviceAllocatorType type, uint64_t size, ID3D12Device* device);
		uint64_t Allocate(uint64_t size);
		[[nodiscard]] ID3D12Heap* GetHeap() const;
	private:
		ID3D12Device* m_device;
		ComPtr<ID3D12Heap> m_heap;
		uint64_t m_currentOffset = 0;
		uint64_t m_size = 0;
	};
}
#endif // DEVICE_LINEAR_ALLOCATOR_H
