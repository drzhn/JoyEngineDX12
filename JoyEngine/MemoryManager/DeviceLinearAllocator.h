#ifndef DEVICE_LINEAR_ALLOCATOR_H
#define DEVICE_LINEAR_ALLOCATOR_H


#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class DeviceLinearAllocator
	{
	public:
		DeviceLinearAllocator() = default;

		DeviceLinearAllocator(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flags, size_t size, ID3D12Device* device);
	private:
		ID3D12Device* m_device;
		ComPtr<ID3D12Heap> m_heap;
	};
}
#endif // DEVICE_LINEAR_ALLOCATOR_H
