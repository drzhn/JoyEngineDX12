#include "DescriptorManager.h"


#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"
#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	DescriptorManager::DescriptorManager()
	{
		TIME_PERF("DescriptorManager ctor")

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

			ComPtr<ID3D12DescriptorHeap> heap;
			ASSERT_SUCC(GraphicsManager::Get()->GetDevice()-> CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap)));

			const auto descriptorSize = GraphicsManager::Get()->GetDevice()->GetDescriptorHandleIncrementSize(type);
			const D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart = heap->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = {};
			if (type != D3D12_DESCRIPTOR_HEAP_TYPE_RTV && type != D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			{
				gpuHeapStart = heap->GetGPUDescriptorHandleForHeapStart();
			}

			m_heapStorage.insert({
				type,
				HeapEntry(heap,
				          descriptorSize,
				          cpuHeapStart,
				          gpuHeapStart,
				          DESCRIPTORS_COUNT)
			});
		}
	}


	void DescriptorManager::AllocateDescriptor(
		const D3D12_DESCRIPTOR_HEAP_TYPE type,
		_Out_ uint32_t& index,
		_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle
	)
	{
		auto& entry = m_heapStorage.at(type);

		ASSERT((entry.m_currentDescriptorIndex + 1) < entry.m_descriptorsCount)

		cpuHandle.ptr = entry.m_cpuHeapStart.ptr + entry.m_currentDescriptorIndex * entry.m_descriptorSize;
		gpuHandle.ptr = entry.m_gpuHeapStart.ptr + entry.m_currentDescriptorIndex * entry.m_descriptorSize;
		index = entry.m_currentDescriptorIndex;

		entry.m_currentDescriptorIndex++;
	}

	void DescriptorManager::GetDescriptorHandleAtIndex(
		const D3D12_DESCRIPTOR_HEAP_TYPE type,
		const uint32_t index,
		_Out_ D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle,
		_Out_ D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle
	) const
	{
		const auto& entry = m_heapStorage.at(type);

		ASSERT((entry.m_currentDescriptorIndex + 1) < entry.m_descriptorsCount)

		cpuHandle.ptr = entry.m_cpuHeapStart.ptr + index * entry.m_descriptorSize;
		gpuHandle.ptr = entry.m_gpuHeapStart.ptr + index * entry.m_descriptorSize;
	}

	void DescriptorManager::FreeDescriptor(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t index)
	{
		// TODO
	}

	void DescriptorManager::PrintStats() const
	{
		Logger::LogFormat("Heap CBV_SRV_UAV allocated %d descriptors\n", m_heapStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).m_currentDescriptorIndex);
		Logger::LogFormat("Heap SAMPLER allocated %d descriptors\n", m_heapStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER).m_currentDescriptorIndex);
		Logger::LogFormat("Heap RTV allocated %d descriptors\n", m_heapStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_RTV).m_currentDescriptorIndex);
		Logger::LogFormat("Heap DSV allocated %d descriptors\n", m_heapStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_DSV).m_currentDescriptorIndex);
	}

	ID3D12DescriptorHeap* DescriptorManager::GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE type) const
	{
		return m_heapStorage.at(type).m_heap.Get();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorManager::GetSRVHeapStartDescriptorHandle() const
	{
		return m_heapStorage.at(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).m_gpuHeapStart;
	}
}
