#ifndef CPU_BUFFER_H
#define CPU_BUFFER_H

#include <array>
#include <memory>

#include "Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	template <typename T>
	class DynamicCpuBuffer
	{
	public:
		DynamicCpuBuffer() = delete;

		explicit DynamicCpuBuffer(uint32_t count):
			m_count(count),
			m_size(((sizeof(T) - 1) / 256 + 1) * 256)
		{
			m_buffers = std::vector<std::unique_ptr<Buffer>>(m_count);
			m_resourceViews = std::vector<std::unique_ptr<ResourceView>>(m_count);

			for (uint32_t i = 0; i < m_count; i++)
			{
				m_buffers[i] = std::make_unique<Buffer>(
					m_size,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					D3D12_HEAP_TYPE_UPLOAD
				);

				D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
					m_buffers[i]->GetBufferResource()->GetGPUVirtualAddress(),
					m_size
				};
				m_resourceViews[i] = std::make_unique<ResourceView>(desc);
			}
		}

		[[nodiscard]] ResourceView* GetView(uint32_t index) const
		{
			return m_resourceViews[index].get();
		}

		void Lock(uint32_t index)
		{
			ASSERT(m_currentLockedArea == nullptr);
			m_currentLockedArea = m_buffers[index]->GetMappedPtr(0, m_size);
		}

		[[nodiscard]] T* GetPtr() const
		{
			ASSERT(m_currentLockedArea != nullptr);
			return static_cast<T*>(m_currentLockedArea->GetMappedPtr());
		}

		void Unlock()
		{
			m_currentLockedArea = nullptr;
		}

		void SetData(const T* dataPtr, uint32_t index)
		{
			Lock(index);
			memcpy(GetPtr(), dataPtr, sizeof(T));
			Unlock();
		}

		~DynamicCpuBuffer() = default;
	private:
		std::unique_ptr<BufferMappedPtr> m_currentLockedArea;

		std::vector<std::unique_ptr<ResourceView>> m_resourceViews;
		std::vector<std::unique_ptr<Buffer>> m_buffers;
		uint32_t m_count;
		uint32_t m_size;
	};
}
#endif // CPU_BUFFER_H
