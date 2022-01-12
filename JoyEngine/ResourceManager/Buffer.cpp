#include "Buffer.h"

#include "JoyContext.h"
#include "GraphicsManager/GraphicsManager.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	// BufferMappedPtr

	BufferMappedPtr::BufferMappedPtr(ComPtr<ID3D12Resource> bufferMemory, uint64_t offset, uint64_t size) :
		m_bufferMemory(bufferMemory)
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

	Buffer::Buffer(uint64_t size, D3D12_RESOURCE_STATES usage, D3D12_HEAP_TYPE properties):
		m_size(size),
		m_usage(usage),
		m_properties(properties)
	{
		const CD3DX12_RESOURCE_DESC bufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_size);

		ASSERT_SUCC(JoyContext::Graphics->GetDevice()->CreateCommittedResource(
			&m_properties,
			D3D12_HEAP_FLAG_NONE,
			&bufferResourceDesc,
			m_usage,
			nullptr,
			IID_PPV_ARGS(&m_buffer)));
	}

	std::unique_ptr<BufferMappedPtr> Buffer::GetMappedPtr(uint64_t offset, uint64_t size) const
	{
		const CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(m_properties);

		ASSERT(m_properties.IsCPUAccessible());
		ASSERT(size <= m_size);

		std::unique_ptr<BufferMappedPtr> ptr = std::make_unique<BufferMappedPtr>(m_buffer, offset, size);
		return std::move(ptr);
	}

	ComPtr<ID3D12Resource> Buffer::GetBuffer() const noexcept
	{
		return m_buffer;
	}
}
