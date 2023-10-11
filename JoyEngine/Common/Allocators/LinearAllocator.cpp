#include "LinearAllocator.h"

#include "Common/Math/MathUtils.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	LinearAllocator::LinearAllocator(uint64_t size, uint64_t alignment):
		m_size(size),
		m_alignment(alignment),
		m_currentOffset(0)
	{
	}

	uint64_t LinearAllocator::Allocate(uint64_t size)
	{
		ASSERT(m_currentOffset + size < m_size);

		const uint64_t oldOffset = m_currentOffset;
		m_currentOffset += jmath::align<uint64_t>(size, m_alignment);
		return oldOffset;
	}
}
