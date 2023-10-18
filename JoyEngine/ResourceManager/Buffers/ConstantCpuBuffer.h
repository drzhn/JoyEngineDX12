#ifndef CONSTANT_BUFFER_H
#define CONSTANT_BUFFER_H

#include <cstdint>
#include <memory>

#include "Buffer.h"
#include "Common/Math/MathUtils.h"
#include "ResourceManager/ResourceView.h"

namespace JoyEngine
{
	template <typename T>
	class ConstantCpuBuffer
	{
	public:
		explicit ConstantCpuBuffer() :
			m_size(jmath::align<uint32_t>(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
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

			const auto mappedPtr = m_buffer->Map();
			const auto ptr = static_cast<T*>(mappedPtr.GetPtr());
			memset(ptr, 0, sizeof(T));

			m_currentLockedArea = m_buffer->Map();
		}

		explicit ConstantCpuBuffer(const T* data) : ConstantCpuBuffer()
		{
			const auto mappedPtr = m_buffer->Map();
			const auto ptr = static_cast<T*>(mappedPtr.GetPtr());
			memcpy(ptr, data, sizeof(T));
		}

		[[nodiscard]] ResourceView* GetView() const
		{
			return m_resourceView.get();
		}

		[[nodiscard]] T* GetPtr()
		{
			return static_cast<T*>(m_currentLockedArea.GetPtr());
		}


		void SetData(T data)
		{
			memcpy(GetPtr(), &data, sizeof(T));
		}

		~ConstantCpuBuffer() = default;

	private:
		std::unique_ptr<Buffer> m_buffer;
		std::unique_ptr<ResourceView> m_resourceView;
		MappedAreaHandle m_currentLockedArea;
		uint32_t m_size;
	};
}
#endif // CONSTANT_BUFFER_H
