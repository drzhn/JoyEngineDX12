#ifndef G_BUFFER_H
#define G_BUFFER_H
#include <memory>

#include "ResourceManager/Texture.h"

struct ID3D12GraphicsCommandList;

namespace JoyEngine
{
	class ResourceView;

	class AbstractGBuffer
	{
	public:
		AbstractGBuffer() = delete;
		AbstractGBuffer(uint32_t width, uint32_t height);

		virtual ~AbstractGBuffer() = default;
		virtual void BarrierColorToRead(ID3D12GraphicsCommandList* commandList) = 0;
		virtual void BarrierColorToWrite(ID3D12GraphicsCommandList* commandList) = 0;
		virtual void BarrierDepthToRead(ID3D12GraphicsCommandList* commandList) = 0;
		virtual void BarrierDepthToWrite(ID3D12GraphicsCommandList* commandList) = 0;

		[[nodiscard]] ResourceView* GetColorSRV() const noexcept { return m_colorTexture->GetSRV(); }
		[[nodiscard]] ResourceView* GetNormalsSRV() const noexcept { return m_normalsTexture->GetSRV(); }
		[[nodiscard]] ResourceView* GetPositionSRV() const noexcept { return m_positionTexture->GetSRV(); }
		[[nodiscard]] ResourceView* GetDepthSRV() const noexcept { return m_depthTexture->GetSRV(); }
	protected:
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		std::unique_ptr<AbstractSingleTexture> m_colorTexture;
		std::unique_ptr<AbstractSingleTexture> m_normalsTexture;
		std::unique_ptr<AbstractSingleTexture> m_positionTexture;
		std::unique_ptr<AbstractSingleTexture> m_depthTexture;
	};

	// for raytracing 
	class UAVGbuffer final : public AbstractGBuffer
	{
	public:
		UAVGbuffer() = delete;
		explicit UAVGbuffer(uint32_t width, uint32_t height);

		void BarrierColorToRead(ID3D12GraphicsCommandList* commandList) override;
		void BarrierColorToWrite(ID3D12GraphicsCommandList* commandList) override;
		void BarrierDepthToRead(ID3D12GraphicsCommandList* commandList) override;
		void BarrierDepthToWrite(ID3D12GraphicsCommandList* commandList) override;

		[[nodiscard]] ResourceView* GetColorUAV() const noexcept { return reinterpret_cast<UAVTexture*>(m_colorTexture.get())->GetUAV(); }
		[[nodiscard]] ResourceView* GetNormalsUAV() const noexcept { return reinterpret_cast<UAVTexture*>(m_normalsTexture.get())->GetUAV(); }
		[[nodiscard]] ResourceView* GetPositionUAV() const noexcept { return reinterpret_cast<UAVTexture*>(m_positionTexture.get())->GetUAV(); }
		[[nodiscard]] ResourceView* GetDepthUAV() const noexcept { return reinterpret_cast<UAVTexture*>(m_depthTexture.get())->GetUAV(); }
		

	private:
	};


	class RTVGbuffer : public AbstractGBuffer
	{
	public:
		RTVGbuffer() = delete;
		explicit  RTVGbuffer(uint32_t width, uint32_t height);

		void BarrierColorToRead(ID3D12GraphicsCommandList* commandList) override;
		void BarrierColorToWrite(ID3D12GraphicsCommandList* commandList) override;
		void BarrierDepthToRead(ID3D12GraphicsCommandList* commandList) override;
		void BarrierDepthToWrite(ID3D12GraphicsCommandList* commandList) override;

		[[nodiscard]] ResourceView* GetColorRTV() const noexcept { return reinterpret_cast<RenderTexture*>(m_colorTexture.get())->GetRTV(); }
		[[nodiscard]] ResourceView* GetNormalsRTV() const noexcept { return reinterpret_cast<RenderTexture*>(m_normalsTexture.get())->GetRTV(); }
		[[nodiscard]] ResourceView* GetPositionRTV() const noexcept { return reinterpret_cast<RenderTexture*>(m_positionTexture.get())->GetRTV(); }
		[[nodiscard]] ResourceView* GetDepthDSV() const noexcept { return reinterpret_cast<DepthTexture*>(m_depthTexture.get())->GetDSV(); }
		

	private:
	};
}

#endif // G_BUFFER_H
