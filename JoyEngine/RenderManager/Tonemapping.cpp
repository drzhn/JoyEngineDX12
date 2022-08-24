#include "Tonemapping.h"

#include "JoyTypes.h"
#include "EngineMaterialProvider/EngineMaterialProvider.h"
#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffer.h"
#include "ResourceManager/DynamicBuffer.h"
#include "ResourceManager/ResourceView.h"
#include "Utils/GraphicsUtils.h"

#include "Utils/GUID.h"

namespace JoyEngine
{
	Tonemapping::Tonemapping(
		uint32_t screenWidth,
		uint32_t screenHeight,
		RenderTexture* hdrRenderTarget,
		DXGI_FORMAT hdrRTVFormat,
		DXGI_FORMAT ldrRTVFormat,
		DXGI_FORMAT depthFormat
	) :
		m_hdrRTVFormat(hdrRTVFormat),
		m_ldrRTVFormat(ldrRTVFormat),
		m_depthFormat(depthFormat),
		m_screenWidth(screenWidth),
		m_screenHeight(screenHeight),
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

			m_hdrToLdrTransitionSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, SharedMaterialArgs>(
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
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT
		);


		m_hdrLuminationBuffer = std::make_unique<Buffer>(
			64 * sizeof(float),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		);

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;

		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer = {
			0,
			64,
			sizeof(float),
			0,
			D3D12_BUFFER_UAV_FLAG_NONE
		};

		m_hdrLuminationBufferUAVView = std::make_unique<ResourceView>(
			uavDesc,
			m_hdrLuminationBuffer->GetBuffer().Get()
		);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer = {
			0,
			64,
			sizeof(float),
			D3D12_BUFFER_SRV_FLAG_NONE
		};
		m_hdrLuminationBufferSRVView = std::make_unique<ResourceView>(
			srvDesc,
			m_hdrLuminationBuffer->GetBuffer().Get()
		);

		m_hdrPrevLuminationBuffer = std::make_unique<Buffer>(
			1 * sizeof(float),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		);
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer = {
			0,
			1,
			sizeof(float),
			0,
			D3D12_BUFFER_UAV_FLAG_NONE
		};
		m_hdrPrevLuminationBufferUAVView = std::make_unique<ResourceView>(
			uavDesc,
			m_hdrPrevLuminationBuffer->GetBuffer().Get()
		);

		m_groupSize = static_cast<uint32_t>(m_screenWidth * m_screenHeight / 16.0f / 1024.0f) + 1;


		const HDRDownScaleConstants downScaleConstants = {
			glm::uvec2(m_screenWidth / 4, m_screenHeight / 4),
			m_screenWidth * m_screenHeight / 16,
			m_groupSize,
			0.01f,
			0.2f
		};

		m_constants = std::make_unique<ConstantBuffer<HDRDownScaleConstants>>(&downScaleConstants);
	}

	void Tonemapping::Render(ID3D12GraphicsCommandList* commandList, const RenderTexture* currentBackBuffer)
	{
		const auto ldrRTVHandle = currentBackBuffer->GetRTV()->GetCPUHandle();

		// First pass
		{
			commandList->SetComputeRootSignature(m_hdrDownscaleFirstPassComputePipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_hdrDownscaleFirstPassComputePipeline->GetPipelineObject().Get());


			GraphicsUtils::AttachViewToCompute(commandList, 0, m_hdrLuminationBufferUAVView.get());
			GraphicsUtils::AttachViewToCompute(commandList, 1, m_hrdDownScaledTexture->GetUAV());
			GraphicsUtils::AttachViewToCompute(commandList, 2, m_hdrRenderTarget->GetSRV());
			GraphicsUtils::AttachViewToCompute(commandList, 3, m_constants->GetView());

			commandList->Dispatch(m_groupSize, 1, 1);
		}

		// Second pass
		{
			commandList->SetComputeRootSignature(m_hdrDownscaleSecondPassComputePipeline->GetRootSignature().Get());
			commandList->SetPipelineState(m_hdrDownscaleSecondPassComputePipeline->GetPipelineObject().Get());

			GraphicsUtils::AttachViewToCompute(commandList, 0, m_hdrLuminationBufferUAVView.get());
			GraphicsUtils::AttachViewToCompute(commandList, 1, m_hdrPrevLuminationBufferUAVView.get());
			GraphicsUtils::AttachViewToCompute(commandList, 2, m_constants->GetView());

			commandList->Dispatch(m_groupSize, 1, 1);
		}

		GraphicsUtils::Barrier(commandList,
		                       m_hdrLuminationBuffer->GetBuffer().Get(),
		                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);

		GraphicsUtils::Barrier(commandList,
		                       m_hrdDownScaledTexture->GetImage().Get(),
		                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		                       D3D12_RESOURCE_STATE_GENERIC_READ);


		// Transition
		{
			GraphicsUtils::SetViewportAndScissor(commandList, m_screenWidth, m_screenHeight);

			commandList->OMSetRenderTargets(
				1,
				&ldrRTVHandle,
				FALSE, nullptr);

			const auto sm = m_hdrToLdrTransitionSharedMaterial;

			commandList->SetPipelineState(sm->GetPipelineObject().Get());
			commandList->SetGraphicsRootSignature(sm->GetRootSignature().Get());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			GraphicsUtils::AttachViewToGraphics(commandList, 0, m_hdrLuminationBufferSRVView.get());
			GraphicsUtils::AttachViewToGraphics(commandList, 1, m_hdrRenderTarget->GetSRV());

			commandList->DrawInstanced(
				3,
				1,
				0, 0);
		}

		GraphicsUtils::Barrier(commandList,
		                       m_hdrLuminationBuffer->GetBuffer().Get(),
		                       D3D12_RESOURCE_STATE_GENERIC_READ,
		                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		GraphicsUtils::Barrier(commandList,
		                       m_hrdDownScaledTexture->GetImage().Get(),
		                       D3D12_RESOURCE_STATE_GENERIC_READ,
		                       D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}
