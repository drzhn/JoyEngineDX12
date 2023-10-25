#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <cstdint>
#include <array>

#include "DynamicCpuBuffer.h"
#include "Common/Allocators/PoolAllocator.h"

namespace JoyEngine
{
	template <typename T, uint32_t Size>
	class DynamicBufferPool
	{
	public:
		DynamicBufferPool() = delete;

		explicit DynamicBufferPool(uint32_t frameCount):
			m_frameCount(frameCount),
			m_buffer(frameCount)
		{
		}

		uint32_t Allocate()
		{
			return m_allocator.Allocate();
		}

		void Free(uint32_t index)
		{
			m_allocator.Free(index);
		}

		void SetValue(uint32_t elementIndex, const T& value)
		{
			m_array[elementIndex] = value;
		}

		T& GetValue(uint32_t elementIndex)
		{
			return m_array[elementIndex];
		}

		void Update(uint32_t frameIndex)
		{
			memcpy(m_buffer.GetPtr(frameIndex, 0), m_array.data(), sizeof(T) * Size);
		}

		DynamicCpuBuffer<T, Size>& GetDynamicBuffer() { return m_buffer; }

	private:
		const uint32_t m_frameCount;
		PoolAllocator<Size> m_allocator;

		std::array<T, Size> m_array;

		DynamicCpuBuffer<T, Size> m_buffer;
	};
}
#endif // OBJECT_POOL_H
