#ifndef DESCRIPTOR_MANAGER_H
#define DESCRIPTOR_MANAGER_H

#include <array>
#include <map>

#include "Common/Singleton.h"


#include <d3d12.h>
#include <memory>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#define DESCRIPTORS_COUNT 2048

namespace JoyEngine
{
	class DescriptorManager : public Singleton<DescriptorManager>
	{
	public:
		DescriptorManager();

		void AllocateDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t& index,
			D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void GetDescriptorHandleAtIndex(
			const D3D12_DESCRIPTOR_HEAP_TYPE type,
			const uint32_t index,
			D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) const;
		void FreeDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t index);

		void PrintStats() const;

		[[nodiscard]] ID3D12DescriptorHeap* GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE) const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHeapStartDescriptorHandle() const;

	private:
		struct HeapEntry
		{
			HeapEntry() = delete;

			HeapEntry(const ComPtr<ID3D12DescriptorHeap>& heap,
			          const uint32_t descriptorSize,
			          const D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart,
			          const D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart,
			          const uint32_t descriptorsCount) :
				m_heap(heap),
				m_descriptorSize(descriptorSize),
				m_cpuHeapStart(cpuHeapStart),
				m_gpuHeapStart(gpuHeapStart),
				m_descriptorsCount(descriptorsCount)
			{
			}

			ComPtr<ID3D12DescriptorHeap> m_heap;
			const uint32_t m_descriptorSize = 0;
			const D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHeapStart = {};
			const D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHeapStart = {};
			const uint32_t m_descriptorsCount = 0;
			uint32_t m_currentDescriptorIndex = 0;
		};

		std::map<D3D12_DESCRIPTOR_HEAP_TYPE, HeapEntry> m_heapStorage;
	};
}

#endif // DESCRIPTOR_MANAGER_H
