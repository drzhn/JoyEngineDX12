#ifndef TONEMAPPING_H
#define TONEMAPPING_H

#include <dxgiformat.h>
#include <memory>

#include "CommonEngineStructs.h"
#include "ResourceManager/ConstantCpuBuffer.h"
#include "ResourceManager/DynamicCpuBuffer.h"
#include "ResourceManager/ResourceHandle.h"


namespace JoyEngine
{
	class RenderTexture;
	class ComputePipeline;
	class SharedMaterial;
	class UAVTexture;
	class Buffer;
	class ResourceView;


	class Tonemapping
	{
	public:
		Tonemapping() = delete;
		explicit Tonemapping(
			uint32_t screenWidth, 
			uint32_t screenHeight, 
			RenderTexture* hdrRenderTarget,
			DXGI_FORMAT hdrRTVFormat, 
			DXGI_FORMAT ldrRTVFormat, 
			DXGI_FORMAT depthFormat
		);
		~Tonemapping() = default;

		void Render(ID3D12GraphicsCommandList* commandList, const RenderTexture* currentBackBuffer);

	private:
		ResourceHandle<ComputePipeline> m_hdrDownscaleFirstPassComputePipeline;
		ResourceHandle<ComputePipeline> m_hdrDownscaleSecondPassComputePipeline;
		ResourceHandle<SharedMaterial> m_hdrToLdrTransitionSharedMaterial;

		std::unique_ptr<UAVTexture> m_hrdDownScaledTexture;
		std::unique_ptr<Buffer> m_hdrLuminationBuffer;
		std::unique_ptr<ResourceView> m_hdrLuminationBufferUAVView;
		std::unique_ptr<ResourceView> m_hdrLuminationBufferSRVView;
		std::unique_ptr<Buffer> m_hdrPrevLuminationBuffer;
		std::unique_ptr<ResourceView> m_hdrPrevLuminationBufferUAVView;

		std::unique_ptr<ConstantCpuBuffer<HDRDownScaleConstants>> m_constants;

		DXGI_FORMAT m_hdrRTVFormat;
		DXGI_FORMAT m_ldrRTVFormat;
		DXGI_FORMAT m_depthFormat;

		uint32_t m_screenWidth;
		uint32_t m_screenHeight;

		RenderTexture* m_hdrRenderTarget;
		uint32_t m_groupSize;
	};
}

#endif // TONEMAPPING_H
