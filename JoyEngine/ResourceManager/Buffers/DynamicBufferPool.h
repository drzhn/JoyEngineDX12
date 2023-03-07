#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <cstdint>
#include <stack>
#include <bitset>

#include "Utils/Assert.h"

namespace JoyEngine
{
	template <typename T, uint32_t Size>
	class DynamicBufferPool
	{
	public:
		DynamicBufferPool()
		{
			for (uint32_t i = 0; i < Size; i++)
			{
				m_freeItems.push(i);
				m_usedItems[i] = false;
			}
		}

		T* Allocate()
		{
			uint32_t index = m_freeItems.pop();
			m_usedItems[index] = true;

			return &m_data[index];
		}

		void Free(T* ptr)
		{
			ASSERT(ptr >= m_data);
			uint32_t index = ptr - m_data;
			ASSERT(index < Size);
			m_freeItems.push(index);
			m_usedItems[index] = false;
		}

	private:
		std::stack<T> m_freeItems;
		std::bitset<Size> m_usedItems;

		T m_data[Size];
	};
}
#endif // OBJECT_POOL_H
