#include "DeviceLinearAllocator.h"

#include "d3dx12.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	DeviceLinearAllocator::DeviceLinearAllocator(D3D12_HEAP_TYPE heapType, D3D12_HEAP_FLAGS flags, size_t size, ID3D12Device* device):
		m_device(device)
	{
		const CD3DX12_HEAP_PROPERTIES properties(heapType);

		D3D12_HEAP_DESC heapDesc{
			size,
			properties,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			flags
		};

		ASSERT_SUCC(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_heap)));
	}
}
