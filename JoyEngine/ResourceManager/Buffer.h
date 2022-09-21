#ifndef BUFFER_H
#define BUFFER_H

#include <memory>

#include "d3dx12.h"
#include "Common/Resource.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class BufferMappedPtr
	{
	public:
		BufferMappedPtr() = delete;
		BufferMappedPtr(ComPtr<ID3D12Resource> bufferMemory, uint64_t offset, uint64_t size);
		~BufferMappedPtr();
		[[nodiscard]] void* GetMappedPtr() const noexcept;
	private:
		void* m_bufferPtr = nullptr;
		ComPtr<ID3D12Resource> m_bufferMemory;
	};

	class Buffer final : public Resource
	{
	public:
		Buffer() = delete;
		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other) = delete;

		explicit Buffer(
			uint64_t size,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties,
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
		);

		~Buffer() override = default;

		[[nodiscard]] std::unique_ptr<BufferMappedPtr> GetMappedPtr(uint64_t offset, uint64_t size) const;
		[[nodiscard]] std::unique_ptr<BufferMappedPtr> GetMappedPtr() const; // whole buffer

		[[nodiscard]] ComPtr<ID3D12Resource> GetBuffer() const noexcept;
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

		[[nodiscard]] D3D12_RESOURCE_STATES GetCurrentResourceState() const noexcept { return m_currentResourceState; }
		void SetCurrentResourceState(D3D12_RESOURCE_STATES newState) noexcept { m_currentResourceState = newState; }
	private:
		uint64_t m_size = 0;
		D3D12_RESOURCE_STATES m_currentResourceState;
		CD3DX12_HEAP_PROPERTIES m_properties;
		ComPtr<ID3D12Resource> m_buffer;
	};
}

#endif
