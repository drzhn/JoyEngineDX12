#ifndef BUFFER_H
#define BUFFER_H

#include <memory>

#include "d3dx12.h"
#include "Common/Resource.h"

using Microsoft::WRL::ComPtr;

namespace JoyEngine
{
	class Buffer;

	class MappedAreaHandle
	{
	public:
		MappedAreaHandle() = default;
		explicit MappedAreaHandle(const Buffer*);
		~MappedAreaHandle();

		MappedAreaHandle(const MappedAreaHandle& other) noexcept = delete;
		MappedAreaHandle(MappedAreaHandle&& other) noexcept;

		MappedAreaHandle& operator=(MappedAreaHandle&&) noexcept;
		MappedAreaHandle& operator=(MappedAreaHandle&) noexcept = delete;

		[[nodiscard]] void* GetPtr() const noexcept;

	private:
		void* m_bufferPtr = nullptr;
		const Buffer* m_buffer = nullptr;

		void Move(MappedAreaHandle&&);
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

		void SetCPUData(const void* dataPtr, uint64_t offset, uint64_t size) const;

		[[nodiscard]] uint64_t GetSizeInBytes() const noexcept { return m_sizeInBytes; }

		[[nodiscard]] MappedAreaHandle Map() const;

		[[nodiscard]] ComPtr<ID3D12Resource> GetBufferResource() const noexcept;
		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

		[[nodiscard]] D3D12_RESOURCE_STATES GetCurrentResourceState() const noexcept { return m_currentResourceState; }
		void SetCurrentResourceState(D3D12_RESOURCE_STATES newState) noexcept { m_currentResourceState = newState; }

	private:
		uint64_t m_sizeInBytes = 0;
		D3D12_RESOURCE_STATES m_currentResourceState;
		CD3DX12_HEAP_PROPERTIES m_properties;
		ComPtr<ID3D12Resource> m_buffer;
	};
}

#endif
