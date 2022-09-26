#ifndef CONSTANT_BUFFER_H
#define CONSTANT_BUFFER_H

#include <cstdint>
#include <memory>

#include "Buffer.h"
#include "ResourceView.h"

namespace JoyEngine
{
	template <typename T>
	class ConstantBuffer
	{
	public:
		explicit ConstantBuffer() :
			m_size(((sizeof(T) - 1) / 256 + 1) * 256)
		{
			m_buffer = std::make_unique<Buffer>(
				m_size,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD
			);

			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
				m_buffer.get()->GetBuffer()->GetGPUVirtualAddress(),
				m_size
			};
			m_resourceView = std::make_unique<ResourceView>(desc);
		}

		explicit ConstantBuffer(const T* data) : ConstantBuffer()
		{
			const auto mappedPtr = m_buffer->GetMappedPtr();
			const auto ptr = static_cast<T*>(mappedPtr->GetMappedPtr());
			memcpy(ptr, data, sizeof(T));
		}

		[[nodiscard]] ResourceView* GetView() const
		{
			return m_resourceView.get();
		}

		void Lock()
		{
			m_currentLockedArea = std::move(m_buffer->GetMappedPtr(0, m_size));
		}

		[[nodiscard]] void* GetPtr() const
		{
			ASSERT(m_currentLockedArea != nullptr);
			return m_currentLockedArea.get()->GetMappedPtr();
		}

		void Unlock()
		{
			m_currentLockedArea = nullptr;
		}

		void SetData(T data)
		{
			Lock();
			memcpy(GetPtr(), &data, sizeof(T));
			Unlock();
		}

		~ConstantBuffer() = default;
	private:
		std::unique_ptr<BufferMappedPtr> m_currentLockedArea;

		std::unique_ptr<ResourceView> m_resourceView;
		std::unique_ptr<Buffer> m_buffer;
		uint32_t m_size;
	};
}
#endif // CONSTANT_BUFFER_H
