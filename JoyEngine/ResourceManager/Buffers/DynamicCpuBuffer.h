#ifndef CPU_BUFFER_H
#define CPU_BUFFER_H

#include <memory>

#include "Buffer.h"
#include "ResourceManager/ResourceView.h"
#include "Utils/Assert.h"
#include "Common/Math/MathUtils.h"

namespace JoyEngine
{
	template <typename T, uint32_t ElemCount = 1>
	class DynamicCpuBuffer
	{
	public:
		DynamicCpuBuffer() = delete;

		explicit DynamicCpuBuffer(const uint32_t frameCount) :
			m_frameCount(frameCount),
			m_alignedStride(jmath::align<uint32_t>(
					sizeof(T),
					ElemCount == 1 ? D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT : 1)
			)
		{
			m_resourceViews = std::vector<std::unique_ptr<ResourceView>>(m_frameCount);

			m_buffer = std::make_unique<Buffer>(
				m_alignedStride * m_frameCount * ElemCount,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				D3D12_HEAP_TYPE_UPLOAD
			);

			if (ElemCount == 1)
			{
				for (uint32_t i = 0; i < m_frameCount; i++)
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
						m_buffer->GetBufferResource()->GetGPUVirtualAddress() + (m_alignedStride * ElemCount) * i,
						m_alignedStride * ElemCount
					};
					m_resourceViews[i] = std::make_unique<ResourceView>(desc);
				}
			}
			else
			{
				for (uint32_t i = 0; i < m_frameCount; i++)
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
						.Format = DXGI_FORMAT_UNKNOWN,
						.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
						.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
						.Buffer = {
							ElemCount * i,
							ElemCount,
							m_alignedStride,
							D3D12_BUFFER_SRV_FLAG_NONE
						}
					};
					m_resourceViews[i] = std::make_unique<ResourceView>(shaderResourceViewDesc, m_buffer->GetBufferResource().Get());
				}
			}

			// keep it always mapped
			m_mappedArea = m_buffer->Map();
		}

		[[nodiscard]] ResourceView* GetView(uint32_t index) const
		{
			return m_resourceViews[index].get();
		}

		[[nodiscard]] T* GetPtr(uint32_t frameIndex, uint32_t elementIndex = 0) const
		{
			ASSERT(m_mappedArea.GetPtr() != nullptr);
			return reinterpret_cast<T*>(
				reinterpret_cast<uint64_t>(
					m_mappedArea.GetPtr()) + m_alignedStride * ElemCount * frameIndex +
				m_alignedStride * elementIndex
			);
		}

		void SetData(const T* dataPtr, uint32_t frameIndex, uint32_t elementIndex = 0)
		{
			memcpy(GetPtr(frameIndex, elementIndex), dataPtr, sizeof(T));
		}

		~DynamicCpuBuffer() = default;

	private:
		std::vector<std::unique_ptr<ResourceView>> m_resourceViews;
		std::unique_ptr<Buffer> m_buffer;
		uint32_t m_frameCount;
		uint32_t m_alignedStride;

		MappedAreaHandle m_mappedArea;
	};
}
#endif // CPU_BUFFER_H
