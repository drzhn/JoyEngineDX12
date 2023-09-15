#ifndef UAV_GPU_BUFFER_H
#define UAV_GPU_BUFFER_H
#include "Buffer.h"
#include "ResourceManager/ResourceView.h"

namespace JoyEngine
{
	class UAVGpuBuffer
	{
	public:
		explicit UAVGpuBuffer(
			uint32_t numElements, 
			size_t stride, 
			D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATE_GENERIC_READ
		);

		[[nodiscard]] ResourceView* GetUAV() const
		{
			return m_bufferViewUAV.get();
		}

		[[nodiscard]] ResourceView* GetSRV() const
		{
			return m_bufferViewSRV.get();
		}

		[[nodiscard]] size_t GetSize() const noexcept { return m_numElements * m_stride; }
		[[nodiscard]] Buffer* GetBuffer() const noexcept { return m_gpuBuffer.get(); }

	protected:
		std::unique_ptr<Buffer> m_gpuBuffer;
		std::unique_ptr<ResourceView> m_bufferViewUAV;
		std::unique_ptr<ResourceView> m_bufferViewSRV;

	private:
		uint32_t m_numElements;
		size_t m_stride;
	};
}
#endif // UAV_GPU_BUFFER_H
