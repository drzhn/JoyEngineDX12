#include "Buffer.h"

#include <utility>

#include "GraphicsManager/GraphicsManager.h"
#include "MemoryManager/MemoryManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	// BufferMappedPtr

	BufferMappedPtr::BufferMappedPtr(ComPtr<ID3D12Resource> bufferMemory, uint64_t offset, uint64_t size) :
		m_bufferMemory(std::move(bufferMemory))
	{
		CD3DX12_RANGE readRange(offset, offset + size);
		ASSERT_SUCC(m_bufferMemory->Map(0, &readRange, &m_bufferPtr))
	}

	BufferMappedPtr::~BufferMappedPtr()
	{
		m_bufferMemory->Unmap(0, nullptr);
	}

	void* BufferMappedPtr::GetMappedPtr() const noexcept
	{
		return m_bufferPtr;
	}

	// Buffer

	Buffer::Buffer(uint64_t size, D3D12_RESOURCE_STATES usage, D3D12_HEAP_TYPE properties, D3D12_RESOURCE_FLAGS flags):
		Resource(GUID::Random()),
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
		const auto ptr = GetMappedPtr(0, m_sizeInBytes);
		memcpy((void*)((size_t)ptr->GetMappedPtr() + offset), dataPtr, size);
	}

	std::unique_ptr<BufferMappedPtr> Buffer::GetMappedPtr(uint64_t offset, uint64_t size) const
	{
		ASSERT(m_properties.IsCPUAccessible());
		ASSERT(offset + size <= m_sizeInBytes);

		// TODO remove std::unique_ptr usage, pass this variables through stack 
		std::unique_ptr<BufferMappedPtr> ptr = std::make_unique<BufferMappedPtr>(m_buffer, offset, size);
		return std::move(ptr);
	}

	std::unique_ptr<BufferMappedPtr> Buffer::GetMappedPtr() const
	{
		return GetMappedPtr(0, m_sizeInBytes);
	}

	ComPtr<ID3D12Resource> Buffer::GetBufferResource() const noexcept
	{
		return m_buffer;
	}
}
