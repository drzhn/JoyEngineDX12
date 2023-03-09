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
			: m_buffer(bufferSize)
		{
			for (uint32_t i = 0; i < Size; i++)
			{
				m_freeItems.push(i);
				m_usedItems[i] = false;
			}
		}

		uint32_t Allocate()
		{
			uint32_t index = m_freeItems.top();
			m_freeItems.pop();
			m_usedItems[index] = true;

			return index;
		}

		void Free(uint32_t index)
		{
			ASSERT(index < Size);
			m_freeItems.push(index);
			m_usedItems[index] = false;
		}


		ElemT& GetElem(uint32_t index) { return m_data[index]; }
		DynamicCpuBuffer<GpuT>& GetDynamicBuffer() { return m_buffer; }
		std::array<ElemT, Size>& GetDataArray() { return m_data; }
		std::bitset<Size>& GetUsedItems() { return m_usedItems; }

	private:
		std::stack<uint32_t> m_freeItems;
		std::bitset<Size> m_usedItems;

		std::array<ElemT, Size> m_data;
		DynamicCpuBuffer<GpuT> m_buffer;
	};
}
#endif // OBJECT_POOL_H
