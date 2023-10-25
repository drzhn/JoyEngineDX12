#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include <bitset>
#include <cstdint>
#include <stack>

#include "Utils/Assert.h"

namespace JoyEngine
{
	template <uint32_t Size>
	class PoolAllocator
	{
	public:
		PoolAllocator()
		{
			for (uint32_t i = 0; i < Size; i++)
			{
				m_freeItems.push(i);
				m_allocatedItems[i] = false;
			}
		}

		uint32_t Allocate()
		{
			uint32_t index = m_freeItems.top();
			m_freeItems.pop();
			m_allocatedItems[index] = true;

			return index;
		}

		void Free(uint32_t index)
		{
			ASSERT(index < Size);
			m_freeItems.push(index);
			m_allocatedItems[index] = false;
		}

	private:
		std::stack<uint32_t> m_freeItems;
		std::bitset<Size> m_allocatedItems;
	};
}

#endif // POOL_ALLOCATOR_H
