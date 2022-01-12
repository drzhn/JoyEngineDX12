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

			entry.descriptorSize = JoyContext::Graphics->GetDevice()->GetDescriptorHandleIncrementSize(
				type);
			entry.heapStart = entry.heap->GetCPUDescriptorHandleForHeapStart();
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorManager::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{

		D3D12_CPU_DESCRIPTOR_HANDLE handle;

		HeapEntry& entry = m_descriptorStorage[type];

		ASSERT(entry.currentDescriptorIndex < DESCRIPTORS_COUNT)

		handle.ptr = entry.heapStart.ptr + entry.currentDescriptorIndex * entry.descriptorSize;

		entry.currentDescriptorIndex++;


		return handle;
	}
}
