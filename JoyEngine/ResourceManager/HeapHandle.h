#ifndef HEAP_HANDLE_H
#define HEAP_HANDLE_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <fstream>
#include <wrl.h>

#include "d3dx12.h"
using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class HeapHandle
	{
	public:
		HeapHandle() = default;
		explicit HeapHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12Resource* resource, DXGI_FORMAT format);
		explicit HeapHandle(D3D12_SAMPLER_DESC);

		[[nodiscard]] D3D12_DESCRIPTOR_HEAP_TYPE GetType() const noexcept { return m_type; }
		[[nodiscard]] ID3D12DescriptorHeap* GetHeap() const noexcept { return m_descriptorHeap.Get(); }
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const noexcept { return m_handle; }
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const noexcept
		{
			return m_descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}
	private:
		D3D12_DESCRIPTOR_HEAP_TYPE m_type;
		ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_handle = {0};
	};
}
#endif // HEAP_HANDLE_H
