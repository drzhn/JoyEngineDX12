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

//#define READONLY_TEXTURES_COUNT 256

namespace JoyEngine
{
	class DescriptorManager : public Singleton<DescriptorManager>
	{
	public:
		DescriptorManager() = default;
		void Init();
		void AllocateDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t& index,
			D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void GetDescriptorHandleAtIndex(
			const D3D12_DESCRIPTOR_HEAP_TYPE type,
			const uint32_t index, 
			D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, 
			D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);
		void FreeDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t index);

		[[nodiscard]] ID3D12DescriptorHeap* GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE) const;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHeapStartDescriptorHandle() const;
	private:
		struct HeapEntry
		{
			HeapEntry(
				uint32_t descriptorSize,
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart,
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart,
				uint32_t descriptorsCount) :
				m_descriptorSize(descriptorSize),
				m_cpuHeapStart(cpuHeapStart),
				m_gpuHeapStart(gpuHeapStart),
				m_descriptorsCount(descriptorsCount)
			{
			}

			const uint32_t m_descriptorSize = 0;
			const D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHeapStart = {};
			const D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHeapStart = {};
			const uint32_t m_descriptorsCount = 0;
			uint32_t m_currentDescriptorIndex = 0;
		};

		std::array<ComPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_heaps;
		std::map<D3D12_DESCRIPTOR_HEAP_TYPE, std::unique_ptr<HeapEntry>> m_descriptorStorage;
	};
}

#endif // DESCRIPTOR_MANAGER_H
