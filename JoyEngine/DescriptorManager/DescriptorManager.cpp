#include "DescriptorManager.h"


#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"
#include "Utils/TimeCounter.h"


namespace JoyEngine
{
	IMPLEMENT_SINGLETON(DescriptorManager)

	D3D12_DESCRIPTOR_HEAP_TYPE GetNativeDescriptorType(DescriptorHeapType type)
	{
		switch (type)
		{
		case DescriptorHeapType::SRV:
		case DescriptorHeapType::CBV_UAV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			break;
		case DescriptorHeapType::SAMPLER:
			return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			break;
		case DescriptorHeapType::RTV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case DescriptorHeapType::DSV:
			return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		default:
			ASSERT(false);
			return static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(-1);
		}
	}

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
		}

		for (uint32_t typeIndex = 0;
		     typeIndex < static_cast<uint32_t>(DescriptorHeapType::NUM_TYPES);
		     typeIndex++)
		{
			const DescriptorHeapType type = static_cast<DescriptorHeapType>(typeIndex);
			const D3D12_DESCRIPTOR_HEAP_TYPE nativeType = GetNativeDescriptorType(type);

			auto descriptorSize = GraphicsManager::Get()->GetDevice()->GetDescriptorHandleIncrementSize(nativeType);
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart = m_heaps[nativeType]->GetCPUDescriptorHandleForHeapStart();
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = m_heaps[nativeType]->GetGPUDescriptorHandleForHeapStart();

			// separate area in descriptor heap for srv and for cbv-uav.
			// [----SRV (SRV_COUNT)----][-----CBV_UAV(DESCRIPTORS_COUNT-SRV_COUNT)----]
			uint32_t count = DESCRIPTORS_COUNT;
			if (type == DescriptorHeapType::SRV)
			{
				count = SRV_COUNT;
			}
			if (type == DescriptorHeapType::CBV_UAV)
			{
				count = DESCRIPTORS_COUNT - SRV_COUNT;

				cpuHeapStart.ptr += SRV_COUNT * descriptorSize;
				gpuHeapStart.ptr += SRV_COUNT * descriptorSize;
			}

			auto entry = std::make_unique<HeapEntry>(
				descriptorSize,
				cpuHeapStart,
				gpuHeapStart,
				count
			);

			m_descriptorStorage[type] = std::move(entry);
		}
	}

	void DescriptorManager::AllocateDescriptor(
		DescriptorHeapType type,
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

	void DescriptorManager::FreeDescriptor(
		DescriptorHeapType type,
		uint32_t index)
	{
		// TODO
	}

	ID3D12DescriptorHeap* DescriptorManager::GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE nativeType) const
	{
		return m_heaps[nativeType].Get();
	}
}
