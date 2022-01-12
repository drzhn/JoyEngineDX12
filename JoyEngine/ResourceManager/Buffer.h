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

		explicit Buffer(
			uint64_t size,
			D3D12_RESOURCE_STATES usage,
			D3D12_HEAP_TYPE properties
		);

		~Buffer() override = default;

		[[nodiscard]] std::unique_ptr<BufferMappedPtr> GetMappedPtr(uint64_t offset, uint64_t size) const;

		[[nodiscard]] ComPtr<ID3D12Resource> GetBuffer() const noexcept;

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }
	private:
		uint64_t m_size = 0;
		D3D12_RESOURCE_STATES m_usage;
		CD3DX12_HEAP_PROPERTIES m_properties;

		ComPtr<ID3D12Resource> m_buffer;
	};
}

#endif
