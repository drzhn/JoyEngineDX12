#include "UAVGpuBuffer.h"

namespace JoyEngine
{
	UAVGpuBuffer::UAVGpuBuffer(
		uint32_t numElements,
		size_t stride,
		D3D12_RESOURCE_STATES resourceStates):
		m_numElements(numElements),
		m_stride(stride)
	{
		m_gpuBuffer = std::make_unique<Buffer>(
			m_numElements * m_stride,
			resourceStates,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc = {
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
			.Buffer = {
				.FirstElement = 0,
				.NumElements = m_numElements,
				.StructureByteStride = static_cast<uint32_t>(stride),
				.CounterOffsetInBytes = 0,
				.Flags = D3D12_BUFFER_UAV_FLAG_NONE,
			}
		};
		m_bufferViewUAV = std::make_unique<ResourceView>(unorderedAccessViewDesc, m_gpuBuffer->GetBufferResource().Get());


		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {
			.Format = DXGI_FORMAT_UNKNOWN,
			.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Buffer = {
				0,
				m_numElements,
				static_cast<uint32_t>(stride),
				D3D12_BUFFER_SRV_FLAG_NONE
			}
		};
		m_bufferViewSRV = std::make_unique<ResourceView>(shaderResourceViewDesc, m_gpuBuffer->GetBufferResource().Get());
	}
}
