#ifndef DESCRIPTOR_MANAGER_H
#define DESCRIPTOR_MANAGER_H

#include <array>
#include <map>

#include "Common/Singleton.h"


#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#define DESCRIPTORS_COUNT 512 // I'm too lazy now for writing pool

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
		void FreeDescriptor(
			D3D12_DESCRIPTOR_HEAP_TYPE type,
			uint32_t index);

		[[nodiscard]] ID3D12DescriptorHeap* GetHeapByType(D3D12_DESCRIPTOR_HEAP_TYPE) const;

	private:
		struct HeapEntry
		{
			HeapEntry(uint32_t stride) :descriptorSize(stride) {}

			ComPtr<ID3D12DescriptorHeap> heap = nullptr;
			const uint32_t descriptorSize = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapStart = {};
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = {};
			uint32_t currentDescriptorIndex = 0;
		};

		std::array<std::unique_ptr<HeapEntry>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_descriptorStorage;
	};
}

#endif // DESCRIPTOR_MANAGER_H
