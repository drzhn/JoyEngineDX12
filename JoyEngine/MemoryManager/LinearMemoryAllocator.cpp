#include "LinearMemoryAllocator.h"

#include "d3dx12.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	LinearMemoryAllocator::LinearMemoryAllocator(D3D12_HEAP_TYPE heapType, DeviceAllocatorType type, uint64_t size, ID3D12Device* device):
		m_device(device),
		m_allocator(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_heap_flags#remarks
		// I have device heap tier 1 on my GTX 1060 3G.
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE;
		switch (type)
		{
		case DeviceAllocatorTypeGpuBuffer:
		case DeviceAllocatorTypeCpuUploadBuffer:
		case DeviceAllocatorTypeCpuReadbackBuffer:
			flags =
				D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES |
				D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
			break;
		case DeviceAllocatorTypeTextures:
			flags =
				D3D12_HEAP_FLAG_DENY_BUFFERS |
				D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
			break;
		case DeviceAllocatorTypeRtDsTextures:
			flags =
				D3D12_HEAP_FLAG_DENY_BUFFERS |
				D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
			break;
		default:
			ASSERT(false);
		}

		const CD3DX12_HEAP_PROPERTIES properties(heapType);

		const D3D12_HEAP_DESC heapDesc{
			size,
			properties,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			flags
		};

		ASSERT_SUCC(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
	}

	uint64_t LinearMemoryAllocator::Allocate(uint64_t size)
	{
		m_unalignedBytesAllocated += size;
		return m_allocator.Allocate(size);
	}

	ID3D12Heap* LinearMemoryAllocator::GetHeap() const
	{
		return m_heap.Get();
	}
}
