#include "Tonemapping.h"

#include "IRenderManager.h"
#include "Common/HashDefs.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/Texture.h"
#include "Utils/GraphicsUtils.h"

#include "Utils/GUID.h"

namespace JoyEngine
{
	Tonemapping::Tonemapping(
		IRenderManager* renderManager,
		RenderTexture* hdrRenderTarget,
		DXGI_FORMAT hdrRTVFormat,
		DXGI_FORMAT ldrRTVFormat,
		DXGI_FORMAT depthFormat
	) :
		m_hdrRTVFormat(hdrRTVFormat),
		m_ldrRTVFormat(ldrRTVFormat),
		m_depthFormat(depthFormat),
		m_screenWidth(renderManager->GetWidth()),
		m_screenHeight(renderManager->GetHeight()),
		m_hdrRenderTarget(hdrRenderTarget)
	{
		// Downscaling first pass
		{
			const GUID hdrDownscaleFirstPassShaderGuid = GUID::StringToGuid("e3e039f4-4f96-4e5b-b90b-1f46d460b724"); //shaders/hdrDownscaleFirstPass.hlsl
			const GUID hdrDownscaleFirstPassPipelineGuid = GUID::Random();

			m_hdrDownscaleFirstPassComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				hdrDownscaleFirstPassPipelineGuid,
				{
					hdrDownscaleFirstPassShaderGuid,
				});
		}

		// Downscaling second pass
		{
			const GUID hdrDownscaleSecondPassShaderGuid = GUID::StringToGuid("c3a1592f-f12d-4c25-bcbb-1e6ace76b0fb"); //shaders/hdrDownscaleSecondPass.hlsl
			const GUID hdrDownscaleSecondPassPipelineGuid = GUID::Random();

			m_hdrDownscaleSecondPassComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				hdrDownscaleSecondPassPipelineGuid,
				{
					hdrDownscaleSecondPassShaderGuid,
				});
		}

		// HDR -> LDR transition
		{
			const GUID hdrToLdrTransitionShaderGuid = GUID::StringToGuid("aa366fc9-b8a7-4cca-b5d3-670216174566"); //shaders/hdrToLdrTransition.hlsl
			const GUID hdrToLdrTransitionSharedMaterialGuid = GUID::Random();

			m_hdrToLdrTransitionGraphicsPipeline = ResourceManager::Get()->LoadResource<GraphicsPipeline, GraphicsPipelineArgs>(
				hdrToLdrTransitionSharedMaterialGuid,
				{
					hdrToLdrTransitionShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						m_ldrRTVFormat
					},
					m_depthFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		m_hrdDownScaledTexture = std::make_unique<UAVTexture>(
			m_screenWidth / 4,
			m_screenHeight / 4,
			hdrRTVFormat,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT
		);


		m_hdrLuminationBuffer = std::make_unique<UAVGpuBuffer>(64, sizeof(float));

		m_hdrPrevLuminationBuffer = std::make_unique<UAVGpuBuffer>(1, sizeof(float));

		m_groupSize = static_cast<uint32_t>(m_screenWidth * m_screenHeight / 16.0f / 1024.0f) + 1;


		m_constantsValues = {
			.Res = glm::uvec2(m_screenWidth / 4, m_screenHeight / 4),
			.Domain = m_screenWidth * m_screenHeight / 16,
			.GroupSize = m_groupSize,
			.AdaptationSpeed = 0.2f,
			.UseGammaCorrection = true,
			.MiddleGrey = 0.9f,
			.LumWhiteSqr = 40.0f,
			.LumFactor = glm::vec3(0.299f, 0.587f, 0.114f),
			.UseTonemapping = true
		};

		m_constantsBuffer = std::make_unique<DynamicCpuBuffer<HDRDownScaleConstants>>(renderManager->GetFrameCount());
		for (uint32_t i = 0; i < renderManager->GetFrameCount(); i++)
		{
			UpdateConstants(i);
		}
	}

	void Tonemapping::Render(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const RenderTexture* currentBackBuffer) const
	{
		// First pass
		{
			const auto& sm = m_hdrDownscaleFirstPassComputePipeline;
			commandList->SetComputeRootSignature(sm->GetRootSignature().Get());
			commandList->SetPipelineState(sm->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("AverageLum")), m_hdrLuminationBuffer->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("HDRDownScale")), m_hrdDownScaledTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("HDRTex")), m_hdrRenderTarget->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("Constants")), m_constantsBuffer->GetView(frameIndex));

			commandList->Dispatch(m_groupSize, 1, 1);
		}

		GraphicsUtils::UAVBarrier(commandList, m_hdrLuminationBuffer->GetBuffer()->GetBufferResource().Get());

		// Second pass
		{
			const auto& sm = m_hdrDownscaleSecondPassComputePipeline;
			commandList->SetComputeRootSignature(sm->GetRootSignature().Get());
			commandList->SetPipelineState(sm->GetPipelineObject().Get());

			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("AverageLum")), m_hdrLuminationBuffer->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("PrevAverageLum")), m_hdrPrevLuminationBuffer->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, sm->GetBindingIndexByHash(strHash("Constants")), m_constantsBuffer->GetView(frameIndex));

			commandList->Dispatch(m_groupSize, 1, 1);
		}
		
		GraphicsUtils::UAVBarrier(commandList, m_hdrLuminationBuffer->GetBuffer()->GetBufferResource().Get());

		// Transition
		{
			const auto& sm = m_hdrToLdrTransitionGraphicsPipeline;

			commandList->SetPipelineState(sm->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			GraphicsUtils::AttachViewToGraphics(commandList, sm->GetBindingIndexByHash(strHash("AvgLum")), m_hdrLuminationBuffer->GetSRV());
			GraphicsUtils::AttachViewToGraphics(commandList, sm->GetBindingIndexByHash(strHash("HdrTexture")), m_hdrRenderTarget->GetSRV());
			GraphicsUtils::AttachViewToGraphics(commandList, sm->GetBindingIndexByHash(strHash("Constants")), m_constantsBuffer->GetView(frameIndex));

			commandList->DrawInstanced(
				3,
				1,
				0, 0);
		}
	}

	void Tonemapping::UpdateConstants(uint32_t frameIndex) const
	{
		HDRDownScaleConstants* ptr = m_constantsBuffer->GetPtr(frameIndex);
		*ptr = m_constantsValues;
	}
}
