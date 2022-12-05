#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <cstdint>

#include "Assert.h"

namespace JoyEngine
{
	template <typename T, uint32_t Size>
	class ObjectStack
	{
	public:
		ObjectStack() = default;

		void Push(T value)
		{
			m_data[m_current] = value;
			m_current++;
			ASSERT(m_current <= Size);
		}

		T Pop()
		{
			m_current--;
			ASSERT(m_current >= 0);

			return m_data[m_current];
		}

	private:
		int32_t m_current = 0;
		T m_data[Size];
	};

	template <typename T, uint32_t Size>
	class ObjectPool
	{
	public:
		ObjectPool()
		{
			for (uint32_t i = 0; i < Size; i++)
			{
				m_freeItems.Push(i);
			}
		}

		T* Get()
		{
			uint32_t index = m_freeItems.Pop();
			return &m_data[index];
		}

		void Return(T* ptr)
		{
			ASSERT(ptr >= m_data);
			uint32_t index = ptr - m_data;
			ASSERT(index < Size);
			m_freeItems.Push(index);
		}

	private:
		ObjectStack<uint32_t, Size> m_freeItems;
		T m_data[Size];
	};
}
#endif // OBJECT_POOL_H
