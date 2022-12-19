#ifndef CONSTANT_BUFFER_H
#define CONSTANT_BUFFER_H

#include <cstdint>
#include <memory>

#include "Buffer.h"
#include "ResourceView.h"

namespace JoyEngine
{
	template <typename T>
	class ConstantCpuBuffer
	{
	public:
		explicit ConstantCpuBuffer() :
			m_size(((sizeof(T) - 1) / 256 + 1) * 256)
		{
			m_buffer = std::make_unique<Buffer>(
				m_size,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD
			);

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
				m_buffer->GetBufferResource()->GetGPUVirtualAddress(),
				m_size
			};
			m_resourceView = std::make_unique<ResourceView>(desc);

			const auto mappedPtr = m_buffer->GetMappedPtr();
			const auto ptr = static_cast<T*>(mappedPtr->GetMappedPtr());
			memset(ptr, 0, sizeof(T));
		}

		explicit ConstantCpuBuffer(const T* data) : ConstantCpuBuffer()
		{
			const auto mappedPtr = m_buffer->GetMappedPtr();
			const auto ptr = static_cast<T*>(mappedPtr->GetMappedPtr());
			memcpy(ptr, data, sizeof(T));
		}

		[[nodiscard]] ResourceView* GetView() const
		{
			return m_resourceView.get();
		}

		void Map()
		{
			m_currentLockedArea = std::move(m_buffer->GetMappedPtr(0, m_size));
		}

		[[nodiscard]] T* GetPtr() 
		{
			ASSERT(m_currentLockedArea != nullptr);
			return static_cast<T*>(m_currentLockedArea->GetMappedPtr());
		}

		void Unmap()
		{
			m_currentLockedArea = nullptr;
		}

		void SetData(T data)
		{
			Map();
			memcpy(GetPtr(), &data, sizeof(T));
			Unmap();
		}

		~ConstantCpuBuffer() = default;
	private:
		std::unique_ptr<BufferMappedPtr> m_currentLockedArea;

		std::unique_ptr<ResourceView> m_resourceView;
		std::unique_ptr<Buffer> m_buffer;
		uint32_t m_size;
	};
}
#endif // CONSTANT_BUFFER_H
