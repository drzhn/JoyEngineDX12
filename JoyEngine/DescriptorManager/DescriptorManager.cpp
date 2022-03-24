#include "DescriptorManager.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"


namespace JoyEngine
{
	void DescriptorManager::Init()
	{
		for (const auto pair : m_descriptorStorage)
		{
			D3D12_DESCRIPTOR_HEAP_TYPE type = pair.first;
			HeapEntry& entry = m_descriptorStorage[type];
			// Describe and create a render target view (RTV) descriptor heap.
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
				type,
				DESCRIPTORS_COUNT,
				type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV
					? D3D12_DESCRIPTOR_HEAP_FLAG_NONE
					: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			ASSERT_SUCC(JoyContext::Graphics->GetDevice()->
				CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&entry.heap)));

			entry.descriptorSize =
				JoyContext::Graphics->GetDevice()->GetDescriptorHandleIncrementSize(type);
			entry.cpuHeapStart = entry.heap->GetCPUDescriptorHandleForHeapStart();
			entry.gpuHeapStart = entry.heap->GetGPUDescriptorHandleForHeapStart();
		}
	}

	void DescriptorManager::AllocateDescriptor(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		_Out_ uint32_t& index,
		_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle
	)
	{
		HeapEntry& entry = m_descriptorStorage[type];

		ASSERT(entry.currentDescriptorIndex < DESCRIPTORS_COUNT)

		cpuHandle.ptr = entry.cpuHeapStart.ptr + entry.currentDescriptorIndex * entry.descriptorSize;
		gpuHandle.ptr = entry.gpuHeapStart.ptr + entry.currentDescriptorIndex * entry.descriptorSize;
		index = entry.currentDescriptorIndex;

		entry.currentDescriptorIndex++;
	}

	void DescriptorManager::FreeDescriptor(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t index)
	{
		// TODO
	}
}
