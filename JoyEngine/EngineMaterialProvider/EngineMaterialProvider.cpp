#include "EngineMaterialProvider.h"

#include <d3d12.h>

#include "d3dx12.h"
#include "RenderManager/JoyTypes.h"
#include "RenderManager/RenderManager.h"

using Microsoft::WRL::ComPtr;

#include "JoyContext.h"

#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	void EngineMaterialProvider::Init()
	{
		Texture::InitSamplers();
		DXGI_FORMAT mainRTVFormat = RenderManager::GetHdrRTVFormat();
		DXGI_FORMAT swapchainLdrFormat = RenderManager::GetLdrRTVFormat();
		DXGI_FORMAT mainGBufferFormat = RenderManager::GetGBufferFormat();
		DXGI_FORMAT mainDSVFormat = RenderManager::GetDepthFormat();
		DXGI_FORMAT ssaoFormat = RenderManager::GetSSAOFormat();


		// Standard shared material
		{
			const GUID standardSharedMaterialGuid = GUID::StringToGuid("b6316780-7043-4ca5-96da-c8bb84042b78"); 
			m_standardSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial>(standardSharedMaterialGuid);
		}

		// Mip map generation
		{
			const GUID mipMapGenerationShaderGuid = GUID::StringToGuid("3fb4d89b-ceab-46c3-b34f-d41a49e072cf"); //shaders/generateMipMaps.hlsl
			const GUID mipMapGenerationPipelineGuid = GUID::Random();

			m_generateMipsComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
				mipMapGenerationPipelineGuid,
				{
					mipMapGenerationShaderGuid,
				});
		}

		// GBuffer write shader
		{
			const GUID gbufferWriteShaderGuid = GUID::StringToGuid("48ffacc9-5c00-4058-b359-cf72189896ac"); //shaders/gbufferwrite.hlsl
			const GUID gbufferWriteSharedMaterialGuid = GUID::Random();

			m_gbufferWriteSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				gbufferWriteSharedMaterialGuid,
				{
					gbufferWriteShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainGBufferFormat,
						mainGBufferFormat,
						mainGBufferFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}


		// Shadow map for spot light creation 
		{
			const GUID shadowProcessingShaderGuid = GUID::StringToGuid("9ee0a40a-c055-4b2c-93db-bc19def8e8cc"); //shaders/shadowprocessing.hlsl
			const GUID shadowProcessingSharedMaterialGuid = GUID::Random();

			m_shadowProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				shadowProcessingSharedMaterialGuid,
				{
					shadowProcessingShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					true,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{}, // no rtv, only depth
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Shadow map for point map creation 
		{
			const GUID shadowPointProcessingShaderGuid = GUID::StringToGuid("9d678808-8c11-4ff3-9ee1-dd1b7fc5f691"); //shaders/shadowpointprocessing.hlsl
			const GUID shadowPointProcessingSharedMaterialGuid = GUID::Random();

			m_shadowPointProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				shadowPointProcessingSharedMaterialGuid,
				{
					shadowPointProcessingShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
					true,
					true,
					true,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{}, // no rtv, only depth
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Direction light processing
		{
			const GUID directionLightProcessingShaderGuid = GUID::StringToGuid("1c6cb88f-f3ef-4797-9d65-44682ca7baba"); //shaders/directionlightprocessing.hlsl
			const GUID directionLightProcessingSharedMaterialGuid = GUID::Random();

			m_directionLightProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				directionLightProcessingSharedMaterialGuid,
				{
					directionLightProcessingShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Light processing
		{
			const GUID lightProcessingShaderGuid = GUID::StringToGuid("f9da7adf-4ebb-4601-8437-a19c07e8471a"); //shaders/lightprocessing.hlsl
			const GUID lightProcessingSharedMaterialGuid = GUID::Random();

			CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				TRUE,FALSE,
				D3D12_BLEND_SRC_COLOR, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			for (auto& desc : blendDesc.RenderTarget)
			{
				desc = defaultRenderTargetBlendDesc;
			}

			m_lightProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				lightProcessingSharedMaterialGuid,
				{
					lightProcessingShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					false,
					D3D12_CULL_MODE_FRONT,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					blendDesc,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// CUBEMAPS
		{
			// Dynamic cubemap reflections
			{
				const GUID dynamicCubemapShaderGUID = GUID::StringToGuid("4a8ea369-904f-4d9a-9061-b4eedacc3918"); // shaders/dynamiccubemapreflections.hlsl
				const GUID sharedMaterialGuid = GUID::Random();

				m_dynamicCubemapReflectionsSharedMaterialHandle = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
					sharedMaterialGuid,
					{
						dynamicCubemapShaderGUID,
						JoyShaderTypeVertex | JoyShaderTypePixel,
						true,
						true,
						true,
						D3D12_CULL_MODE_BACK,
						D3D12_COMPARISON_FUNC_LESS_EQUAL,
						CD3DX12_BLEND_DESC(D3D12_DEFAULT),
						{
							mainRTVFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}

			// Cubemap convolution 
			{
				const GUID cubemapConvolutionShaderGuid = GUID::StringToGuid("613710d9-304e-453b-a655-b1c842904f4c"); //shaders/dynamiccubemapconvolition.hlsl
				const GUID cubemapConvolutionSharedMaterialGuid = GUID::Random();

				m_cubemapConvolutionSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
					cubemapConvolutionSharedMaterialGuid,
					{
						cubemapConvolutionShaderGuid,
						JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
						true,
						false,
						false,
						D3D12_CULL_MODE_NONE,
						D3D12_COMPARISON_FUNC_LESS_EQUAL,
						CD3DX12_BLEND_DESC(D3D12_DEFAULT),
						{
							mainRTVFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}
		}

		// Particle system drawing
		{
			const GUID shaderGuid = GUID::StringToGuid("a36fff56-b183-418a-9bd1-31cffd247e37"); // shaders/particles.hlsl
			const GUID sharedMaterialGuid = GUID::Random();

			m_particleSystemSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sharedMaterialGuid,
				{
					shaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
					false,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
				});
		}

		// Particle system buffer generation
		{
			const GUID bufferGenerationShaderGuid = GUID::StringToGuid("38d4f011-405a-4602-8f8e-79b4888d26b6"); //shaders/particlesbuffergeneration.hlsl
			const GUID bufferGenerationPipelineGuid = GUID::Random();

			m_particleBufferGenerationComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
				bufferGenerationPipelineGuid,
				{
					bufferGenerationShaderGuid,
				});
		}

		// Fog post-effect
		{
			const GUID fogPostEffectShaderGuid = GUID::StringToGuid("5e897d4b-2ed5-4176-890b-2f17e52cb836"); //shaders/fogpostprocess.hlsl
			const GUID fogPostEffectSharedMaterialGuid = GUID::Random();

			m_fogPostProcessSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				fogPostEffectSharedMaterialGuid,
				{
					fogPostEffectShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// SSLR post-effect
		{
			const GUID sslrPostEffectShaderGuid = GUID::StringToGuid("70d5a60a-615a-4344-977f-d2ad9d37b0cf"); //shaders/sslrpostprocess.hlsl
			const GUID sslrPostEffectSharedMaterialGuid = GUID::Random();

			m_sslrPostProcessSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sslrPostEffectSharedMaterialGuid,
				{
					sslrPostEffectShaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// SSAO
		{
			// SSAO Generation
			{
				const GUID ssaoPostEffectShaderGuid = GUID::StringToGuid("3cf96278-de88-4614-bb23-5cfa2b54e41a"); //shaders/ssaopostprocess.hlsl
				const GUID ssaoPostEffectSharedMaterialGuid = GUID::Random();

				m_ssaoPostProcessSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
					ssaoPostEffectSharedMaterialGuid,
					{
						ssaoPostEffectShaderGuid,
						JoyShaderTypeVertex | JoyShaderTypePixel,
						false,
						false,
						false,
						D3D12_CULL_MODE_NONE,
						D3D12_COMPARISON_FUNC_GREATER_EQUAL,
						CD3DX12_BLEND_DESC(D3D12_DEFAULT),
						{
							ssaoFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}

			// SSAO Blur pass
			{
				const GUID ssaoBlurShaderGuid = GUID::StringToGuid("81312dae-c920-4cd6-bd08-87d1a2a3b6f4"); //shaders/ssaoblur.hlsl
				const GUID ssaoBlurSharedMaterialGuid = GUID::Random();

				m_ssaoBlurSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
					ssaoBlurSharedMaterialGuid,
					{
						ssaoBlurShaderGuid,
						JoyShaderTypeVertex | JoyShaderTypePixel,
						false,
						false,
						false,
						D3D12_CULL_MODE_NONE,
						D3D12_COMPARISON_FUNC_GREATER_EQUAL,
						CD3DX12_BLEND_DESC(D3D12_DEFAULT),
						{
							ssaoFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}

			// SSAO append
			{
				const GUID ssaoAppendShaderGuid = GUID::StringToGuid("c7986e89-5d7e-4348-903e-761f007c3f12"); //shaders/ssaoappend.hlsl
				const GUID ssaoAppendSharedMaterialGuid = GUID::Random();

				m_ssaoAppendSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
					ssaoAppendSharedMaterialGuid,
					{
						ssaoAppendShaderGuid,
						JoyShaderTypeVertex | JoyShaderTypePixel,
						false,
						false,
						false,
						D3D12_CULL_MODE_NONE,
						D3D12_COMPARISON_FUNC_GREATER_EQUAL,
						CD3DX12_BLEND_DESC(D3D12_DEFAULT),
						{
							mainRTVFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}
		}

		// Bloom
		{
			// Bright pass
			{
				const GUID bloomBrightPassShaderGuid = GUID::StringToGuid("110f9d5d-008a-4f07-8a7c-e0a208353bf1"); //shaders/bloomBrightPass.hlsl
				const GUID bloomBrightPassPipelineGuid = GUID::Random();

				m_bloomBrightPassComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					bloomBrightPassPipelineGuid,
					{
						bloomBrightPassShaderGuid,
					});
			}
			// Vertical filter
			{
				const GUID verticalFilterShaderGuid = GUID::StringToGuid("79791a5c-5374-4e44-9ad0-f2cf57e7c8e6"); //shaders/bloomVerticalFilter.hlsl
				const GUID verticalFilterPipelineGuid = GUID::Random();

				m_bloomVerticalFilterComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					verticalFilterPipelineGuid,
					{
						verticalFilterShaderGuid,
					});
			}
			// Horizontal filter
			{
				const GUID horizontalFilterShaderGuid = GUID::StringToGuid("fcdfbb5b-72ea-4b7b-b432-3f807ffc576d"); //shaders/bloomHorizontalFilter.hlsl
				const GUID horizontalFilterPipelineGuid = GUID::Random();

				m_bloomHorizontalFilterComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					horizontalFilterPipelineGuid,
					{
						horizontalFilterShaderGuid,
					});
			}
		}

		// HDR
		{
			// Downscaling first pass
			{
				const GUID hdrDownscaleFirstPassShaderGuid = GUID::StringToGuid("e3e039f4-4f96-4e5b-b90b-1f46d460b724"); //shaders/hdrDownscaleFirstPass.hlsl
				const GUID hdrDownscaleFirstPassPipelineGuid = GUID::Random();

				m_hdrDownscaleFirstPassComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					hdrDownscaleFirstPassPipelineGuid,
					{
						hdrDownscaleFirstPassShaderGuid,
					});
			}

			// Downscaling second pass
			{
				const GUID hdrDownscaleSecondPassShaderGuid = GUID::StringToGuid("c3a1592f-f12d-4c25-bcbb-1e6ace76b0fb"); //shaders/hdrDownscaleSecondPass.hlsl
				const GUID hdrDownscaleSecondPassPipelineGuid = GUID::Random();

				m_hdrDownscaleSecondPassComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					hdrDownscaleSecondPassPipelineGuid,
					{
						hdrDownscaleSecondPassShaderGuid,
					});
			}

			// HDR -> LDR transition
			{
				const GUID hdrToLdrTransitionShaderGuid = GUID::StringToGuid("aa366fc9-b8a7-4cca-b5d3-670216174566"); //shaders/hdrToLdrTransition.hlsl
				const GUID hdrToLdrTransitionSharedMaterialGuid = GUID::Random();

				m_hdrToLdrTransitionSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
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
							swapchainLdrFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					});
			}
		}
	}
}
