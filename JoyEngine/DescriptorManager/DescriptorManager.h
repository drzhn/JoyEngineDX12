#ifndef DESCRIPTOR_MANAGER_H
#define DESCRIPTOR_MANAGER_H

#include <map>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
#define DESCRIPTORS_COUNT 512 // I'm too lazy now for writing pool

	class DescriptorManager
	{
	public :
		DescriptorManager() = default;
		void Init();
		void AllocateDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t& index,
			D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void FreeDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t index);

		ID3D12DescriptorHeap* GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE) const;

	private:
		struct HeapEntry
		{
			ComPtr<ID3D12DescriptorHeap> heap;
			uint32_t descriptorSize;
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart;
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart;
			uint32_t currentDescriptorIndex = 0;
		};

		std::map<D3D12_DESCRIPTOR_HEAP_TYPE, HeapEntry> m_descriptorStorage = {
			{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, {}},
			{D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, {}},
			{D3D12_DESCRIPTOR_HEAP_TYPE_RTV, {}},
			{D3D12_DESCRIPTOR_HEAP_TYPE_DSV, {}}
		};
	};
}

#endif // DESCRIPTOR_MANAGER_H
