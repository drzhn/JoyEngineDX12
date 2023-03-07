#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <cstdint>
#include <stack>
#include <bitset>

#include "DynamicCpuBuffer.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	template <typename ElemT, typename GpuT, uint32_t Size>
	class DynamicBufferPool
	{
	public:
		explicit DynamicBufferPool(uint32_t bufferSize)
		{
			static_assert(sizeof(GpuT) == (sizeof(ElemT) * Size));

			for (uint32_t i = 0; i < Size; i++)
			{
				m_freeItems.push(i);
				m_usedItems[i] = false;
			}
			m_buffer = std::make_unique<DynamicCpuBuffer<GpuT>>(bufferSize);
		}

		ElemT* Allocate(uint32_t& allocatedIndex)
		{
			uint32_t index = m_freeItems.top();
			m_freeItems.pop();
			m_usedItems[index] = true;
			allocatedIndex = index;

			return &m_data[index];
		}

		void Free(ElemT* ptr)
		{
			ASSERT(ptr >= m_data);
			uint32_t index = ptr - m_data;
			ASSERT(index < Size);
			m_freeItems.push(index);
			m_usedItems[index] = false;
		}

		void Update(uint32_t bufferIndex)
		{
			m_buffer->SetData(m_data, bufferIndex);
		}

	private:
		std::stack<uint32_t> m_freeItems;
		std::bitset<Size> m_usedItems;

		std::array<ElemT, Size> m_data;
		std::unique_ptr<DynamicCpuBuffer<GpuT>> m_buffer;
	};
}
#endif // OBJECT_POOL_H
