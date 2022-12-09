#include "GBuffer.h"

#include "RenderManager.h"
#include "Utils/GraphicsUtils.h"

namespace JoyEngine
{
	AbstractGBuffer::AbstractGBuffer(uint32_t width, uint32_t height):
		m_width(width),
		m_height(height)
	{
	}

	UAVGbuffer::UAVGbuffer(uint32_t width, uint32_t height): AbstractGBuffer(width, height)
	{
		m_colorTexture = std::make_unique<UAVTexture>(
			m_width,
			m_height,
			RenderManager::GetGBufferFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_normalsTexture = std::make_unique<UAVTexture>(
			m_width,
			m_height,
			RenderManager::GetGBufferFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);


		m_depthTexture = std::make_unique<UAVTexture>(
			m_width,
			m_height,
			RenderManager::GetDepthUAVFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);
	}

	void UAVGbuffer::BarrierToRead(ID3D12GraphicsCommandList* commandList)
	{
		GraphicsUtils::UAVBarrier(commandList, m_colorTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, m_normalsTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, m_depthTexture->GetImageResource().Get());
	}

	void UAVGbuffer::BarrierToWrite(ID3D12GraphicsCommandList* commandList)
	{
		GraphicsUtils::UAVBarrier(commandList, m_colorTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, m_normalsTexture->GetImageResource().Get());
		GraphicsUtils::UAVBarrier(commandList, m_depthTexture->GetImageResource().Get());
	}

	RTVGbuffer::RTVGbuffer(uint32_t width, uint32_t height) : AbstractGBuffer(width, height)
	{
		m_colorTexture = std::make_unique<RenderTexture>(
			m_width,
			m_height,
			RenderManager::GetGBufferFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_normalsTexture = std::make_unique<RenderTexture>(
			m_width,
			m_height,
			RenderManager::GetGBufferFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);

		m_depthTexture = std::make_unique<DepthTexture>(
			m_width,
			m_height,
			RenderManager::GetDepthFormat(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);
	}

	void RTVGbuffer::BarrierToRead(ID3D12GraphicsCommandList* commandList)
	{
		GraphicsUtils::Barrier(commandList, m_colorTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		GraphicsUtils::Barrier(commandList, m_normalsTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
		GraphicsUtils::Barrier(commandList, m_depthTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void RTVGbuffer::BarrierToWrite(ID3D12GraphicsCommandList* commandList)
	{
		GraphicsUtils::Barrier(commandList, m_colorTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GraphicsUtils::Barrier(commandList, m_normalsTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GraphicsUtils::Barrier(commandList, m_depthTexture->GetImageResource().Get(),
		                       D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}
