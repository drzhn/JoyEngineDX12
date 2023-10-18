#include "Buffer.h"

#include <utility>

#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	// MappedAreaHandle

	MappedAreaHandle::MappedAreaHandle(const Buffer* buffer) :
		m_buffer(buffer)
	{
		const D3D12_RANGE readRange{
			.Begin = 0,
			.End = m_buffer->GetSizeInBytes()
		};
		ASSERT_SUCC(m_buffer->GetBufferResource()->Map(0, &readRange, &m_bufferPtr))
	}

	MappedAreaHandle::~MappedAreaHandle()
	{
		if (m_buffer == nullptr) return;
		m_buffer->GetBufferResource()->Unmap(0, nullptr);

		m_bufferPtr = nullptr;
		m_buffer = nullptr;
	}

	MappedAreaHandle::MappedAreaHandle(MappedAreaHandle&& other) noexcept
	{
		Move(std::forward<MappedAreaHandle>(other));
	}

	MappedAreaHandle& MappedAreaHandle::operator=(MappedAreaHandle&& other) noexcept
	{
		Move(std::forward<MappedAreaHandle>(other));
		return *this;
	}

	void* MappedAreaHandle::GetPtr() const noexcept
	{
		ASSERT(m_bufferPtr != nullptr);
		return m_bufferPtr;
	}

	void MappedAreaHandle::Move(MappedAreaHandle&& other)
	{
		m_buffer = other.m_buffer;
		m_bufferPtr = other.m_bufferPtr;

		other.m_buffer = nullptr;
		other.m_bufferPtr = nullptr;
	}

	// Buffer

	Buffer::Buffer(uint64_t size, D3D12_RESOURCE_STATES usage, D3D12_HEAP_TYPE properties, D3D12_RESOURCE_FLAGS flags):
		Resource(RandomHash64()),
		m_sizeInBytes(size),
		m_currentResourceState(usage),
		m_properties(properties)
	{
		ASSERT(m_sizeInBytes != 0);
		const CD3DX12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_sizeInBytes, flags);

		m_buffer = MemoryManager::Get()->CreateResource(
			properties,
			&bufferResourceDesc,
			m_currentResourceState);
	}

	void Buffer::SetCPUData(const void* dataPtr, uint64_t offset, uint64_t size) const
	{
		const auto ptr = Map();
		memcpy((void*)((size_t)ptr.GetPtr() + offset), dataPtr, size);
	}

	MappedAreaHandle Buffer::Map() const
	{
		ASSERT(m_properties.IsCPUAccessible());

		MappedAreaHandle handle = MappedAreaHandle(this);
		return handle;
	}

	ComPtr<ID3D12Resource> Buffer::GetBufferResource() const noexcept
	{
		return m_buffer;
	}
}
