#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

#include <cstdint>

namespace JoyEngine
{
	class LinearAllocator
	{
	public:
		LinearAllocator() = delete;
		LinearAllocator(uint64_t size, uint64_t alignment);
		uint64_t Allocate(uint64_t size);

		[[nodiscard]] uint64_t GetSize() const { return m_size; }
		[[nodiscard]] uint64_t GetAlignment() const { return m_alignment; }
		[[nodiscard]] uint64_t GetCurrentOffset() const { return m_currentOffset; }

	private:
		const uint64_t m_size;
		const uint64_t m_alignment;
		uint64_t m_currentOffset;
	};
}

#endif // LINEAR_ALLOCATOR_H
