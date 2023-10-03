#include "DescriptorManager.h"


#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"
#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	void DescriptorManager::Init()
	{
		TIME_PERF("DescriptorManager init")

		for (uint32_t typeIndex = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		     typeIndex < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
		     typeIndex++)
		{
			const auto type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(typeIndex);
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {
				type,
				DESCRIPTORS_COUNT,
				type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV
					? D3D12_DESCRIPTOR_HEAP_FLAG_NONE
					: D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
				0
			};
			ASSERT_SUCC(GraphicsManager::Get()->GetDevice()->
				CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_heaps[type])));

			auto descriptorSize = GraphicsManager::Get()->GetDevice()->GetDescriptorHandleIncrementSize(type);
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart = m_heaps[type]->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = {};
			if (type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			{
				gpuHeapStart = m_heaps[type]->GetGPUDescriptorHandleForHeapStart();
			}

			auto entry = std::make_unique<HeapEntry>(
				descriptorSize,
				cpuHeapStart,
				gpuHeapStart,
				DESCRIPTORS_COUNT
			);

			m_descriptorStorage[type] = std::move(entry);
		}
	}

	void DescriptorManager::AllocateDescriptor(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		_Out_ uint32_t& index,
		_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle
	)
	{
		const auto entry = m_descriptorStorage[type].get();

		ASSERT((entry->m_currentDescriptorIndex + 1) < entry->m_descriptorsCount)

		cpuHandle.ptr = entry->m_cpuHeapStart.ptr + entry->m_currentDescriptorIndex * entry->m_descriptorSize;
		gpuHandle.ptr = entry->m_gpuHeapStart.ptr + entry->m_currentDescriptorIndex * entry->m_descriptorSize;
		index = entry->m_currentDescriptorIndex;

		entry->m_currentDescriptorIndex++;
	}

	void DescriptorManager::GetDescriptorHandleAtIndex(
		const D3D12_DESCRIPTOR_HEAP_TYPE type,
		const uint32_t index,
		_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle
	)
	{
		const auto entry = m_descriptorStorage[type].get();

		ASSERT((entry->m_currentDescriptorIndex + 1) < entry->m_descriptorsCount)

		cpuHandle.ptr = entry->m_cpuHeapStart.ptr + index * entry->m_descriptorSize;
		gpuHandle.ptr = entry->m_gpuHeapStart.ptr + index * entry->m_descriptorSize;
	}

	void DescriptorManager::FreeDescriptor(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t index)
	{
		// TODO
	}

	ID3D12DescriptorHeap* DescriptorManager::GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE typeIndex) const
	{
		return m_heaps[typeIndex].Get();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorManager::GetSRVHeapStartDescriptorHandle() const
	{
		return m_descriptorStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).get()->m_gpuHeapStart;
	}
}
