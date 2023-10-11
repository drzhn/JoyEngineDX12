#ifndef CPU_BUFFER_H
#define CPU_BUFFER_H

#include <memory>

#include "Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "Utils/Assert.h"
#include "Common/Math/MathUtils.h"

namespace JoyEngine
{
	template <typename T>
	class DynamicCpuBuffer
	{
	public:
		DynamicCpuBuffer() = delete;

		explicit DynamicCpuBuffer(uint32_t count):
			m_count(count),
			m_alignedStride(jmath::align<uint32_t>(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT))
		{
			m_resourceViews = std::vector<std::unique_ptr<ResourceView>>(m_count);

			m_buffer = std::make_unique<Buffer>(
				m_alignedStride * m_count,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD
			);

			for (uint32_t i = 0; i < m_count; i++)
			{
				D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
					m_buffer->GetBufferResource()->GetGPUVirtualAddress() + m_alignedStride * i,
					m_alignedStride
				};
				m_resourceViews[i] = std::make_unique<ResourceView>(desc);
			}

			// keep it always mapped
			m_mappedArea = m_buffer->Map();
		}

		[[nodiscard]] ResourceView* GetView(uint32_t index) const
		{
			return m_resourceViews[index].get();
		}

		[[nodiscard]] T* GetPtr(uint32_t index) const
		{
			ASSERT(m_mappedArea.GetPtr() != nullptr);
			return reinterpret_cast<T*>(reinterpret_cast<uint64_t>(m_mappedArea.GetPtr()) + m_alignedStride * index);
		}

		void SetData(const T* dataPtr, uint32_t index)
		{
			memcpy(GetPtr(index), dataPtr, sizeof(T));
		}

		~DynamicCpuBuffer() = default;

	private:
		std::vector<std::unique_ptr<ResourceView>> m_resourceViews;
		std::unique_ptr<Buffer> m_buffer;
		uint32_t m_count;
		uint32_t m_alignedStride;
		MappedAreaHandle m_mappedArea;
	};
}
#endif // CPU_BUFFER_H
