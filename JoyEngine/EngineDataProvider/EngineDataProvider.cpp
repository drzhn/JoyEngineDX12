#include "EngineDataProvider.h"

#include <d3d12.h>

#include "d3dx12.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/RenderManager.h"
#include "Utils/TimeCounter.h"
#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	void EngineDataProvider::Init()
	{
		TIME_PERF("EngineDataProvider init")

		EngineSamplersProvider::InitSamplers();
		DXGI_FORMAT mainRTVFormat = RenderManager::Get()->GetHdrRTVFormat();
		DXGI_FORMAT swapchainLdrFormat = RenderManager::Get()->GetSwapchainFormat();
		DXGI_FORMAT mainGBufferFormat = RenderManager::Get()->GetGBufferFormat();
		DXGI_FORMAT mainDSVFormat = RenderManager::Get()->GetDepthFormat();
		DXGI_FORMAT ssaoFormat = RenderManager::Get()->GetSSAOFormat();


		// Standard shared material
		{
			m_standardSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial>(
				"shared_materials/standardSharedMaterial.json"
			);
		}


		//// Mip map generation
		//{
		//	const uint64_t mipMapGenerationShaderGuid = uint64_t::StringToGuid("shaders/generateMipMaps.hlsl"); //shaders/generateMipMaps.hlsl
		//	const uint64_t mipMapGenerationPipelineGuid = uint64_t::Random();

		//	m_generateMipsComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//		mipMapGenerationPipelineGuid,
		//		{
		//			mipMapGenerationShaderGuid,
		//		});
		//}

		// Gizmo Axis draw 
		{
			m_gizmoAxisDrawerSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
				RandomHash64(),
				{
					"shaders/gizmoAxisDrawer.hlsl",
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_NEVER,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						swapchainLdrFormat
					},
					1,
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
				});
		}

		// Null texture view 
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D = {
					.MostDetailedMip = 0,
					.MipLevels = 1,
					.PlaneSlice = 0,
					.ResourceMinLODClamp = 0
				}
			};
			m_nullTextureView = std::make_unique<ResourceView>(desc, nullptr);
			ASSERT(m_nullTextureView->GetDescriptorIndex() == 0)

			for (uint32_t i = 1; i < DESCRIPTORS_COUNT; i++)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
				DescriptorManager::Get()->GetDescriptorHandleAtIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, i, cpuHandle, gpuHandle);

				GraphicsManager::Get()->GetDevice()->CreateShaderResourceView(
					nullptr,
					&desc,
					cpuHandle);
			}
		}

		// Materials data that we will pass to GPU
		{
			m_materials = std::make_unique<ConstantCpuBuffer<StandardMaterialData>>();
		}

		{
			m_engineDataBuffer = std::make_unique<DynamicCpuBuffer<EngineData>>(RenderManager::Get()->GetFrameCount());
		}

		{
			m_meshContainer = std::make_unique<MeshContainer>();
		}


		// GBuffer write shader
		{
			m_gbufferWriteSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
				RandomHash64(),
				{
					"shaders/gbufferwrite.hlsl",
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
						mainGBufferFormat,
					},
					3,
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		// Deferred shading processing
		{
			m_deferredShadingProcessorSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
				RandomHash64(),
				{
					"shaders/deferredshadingprocessor.hlsl",
					JoyShaderTypeVertex | JoyShaderTypePixel,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{
						mainGBufferFormat,
					},
					1,
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}


		// Shadow map for spot and directional light creation 
		{
			m_shadowProcessingSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
				RandomHash64(),
				{
					"shaders/shadowprocessing.hlsl",
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					true,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					{}, // no rtv, only depth
					0,
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				});
		}

		//// Shadow map for point map creation 
		//{
		//	const uint64_t shadowPointProcessingShaderGuid = uint64_t::StringToGuid("shaders/shadowpointprocessing.hlsl"); //shaders/shadowpointprocessing.hlsl
		//	const uint64_t shadowPointProcessingSharedMaterialGuid = uint64_t::Random();
		//	m_shadowPointProcessingSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		shadowPointProcessingSharedMaterialGuid,
		//		{
		//			shadowPointProcessingShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
		//			true,
		//			true,
		//			true,
		//			D3D12_CULL_MODE_NONE,
		//			D3D12_COMPARISON_FUNC_LESS_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{}, // no rtv, only depth
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}
		//// Direction light processing
		//{
		//	const uint64_t directionLightProcessingShaderGuid = uint64_t::StringToGuid("shaders/directionlightprocessing.hlsl"); //shaders/directionlightprocessing.hlsl
		//	const uint64_t directionLightProcessingSharedMaterialGuid = uint64_t::Random();
		//	m_directionLightProcessingSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		directionLightProcessingSharedMaterialGuid,
		//		{
		//			directionLightProcessingShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
		//			false,
		//			false,
		//			false,
		//			D3D12_CULL_MODE_NONE,
		//			D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{
		//				mainRTVFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}
		//// Light processing
		//{
		//	const uint64_t lightProcessingShaderGuid = uint64_t::StringToGuid("shaders/lightprocessing.hlsl"); //shaders/lightprocessing.hlsl
		//	const uint64_t lightProcessingSharedMaterialGuid = uint64_t::Random();
		//	CD3DX12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
		//	{
		//		TRUE,FALSE,
		//		D3D12_BLEND_SRC_COLOR, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
		//		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		//		D3D12_LOGIC_OP_NOOP,
		//		D3D12_COLOR_WRITE_ENABLE_ALL,
		//	};
		//	for (auto& desc : blendDesc.RenderTarget)
		//	{
		//		desc = defaultRenderTargetBlendDesc;
		//	}
		//	m_lightProcessingSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		lightProcessingSharedMaterialGuid,
		//		{
		//			lightProcessingShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
		//			true,
		//			true,
		//			false,
		//			D3D12_CULL_MODE_FRONT,
		//			D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//			blendDesc,
		//			{
		//				mainRTVFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}
		//// CUBEMAPS
		//{
		//	// Dynamic cubemap reflections
		//	{
		//		const uint64_t dynamicCubemapShaderGUID = uint64_t::StringToGuid("shaders/dynamiccubemapreflections.hlsl"); // shaders/dynamiccubemapreflections.hlsl
		//		const uint64_t sharedMaterialGuid = uint64_t::Random();
		//		m_dynamicCubemapReflectionsSharedMaterialHandle = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//			sharedMaterialGuid,
		//			{
		//				dynamicCubemapShaderGUID,
		//				JoyShaderTypeVertex | JoyShaderTypePixel,
		//				true,
		//				true,
		//				true,
		//				D3D12_CULL_MODE_BACK,
		//				D3D12_COMPARISON_FUNC_LESS_EQUAL,
		//				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//				{
		//					mainRTVFormat
		//				},
		//				mainDSVFormat,
		//				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//			});
		//	}
		//	// Cubemap convolution 
		//	{
		//		const uint64_t cubemapConvolutionShaderGuid = uint64_t::StringToGuid("shaders/dynamiccubemapconvolition.hlsl"); //shaders/dynamiccubemapconvolition.hlsl
		//		const uint64_t cubemapConvolutionSharedMaterialGuid = uint64_t::Random();
		//		m_cubemapConvolutionSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//			cubemapConvolutionSharedMaterialGuid,
		//			{
		//				cubemapConvolutionShaderGuid,
		//				JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
		//				true,
		//				false,
		//				false,
		//				D3D12_CULL_MODE_NONE,
		//				D3D12_COMPARISON_FUNC_LESS_EQUAL,
		//				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//				{
		//					mainRTVFormat
		//				},
		//				mainDSVFormat,
		//				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//			});
		//	}
		//}
		//// Particle system drawing
		//{
		//	const uint64_t shaderGuid = uint64_t::StringToGuid("shaders/particles.hlsl"); // shaders/particles.hlsl
		//	const uint64_t sharedMaterialGuid = uint64_t::Random();
		//	m_particleSystemSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		sharedMaterialGuid,
		//		{
		//			shaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel | JoyShaderTypeGeometry,
		//			false,
		//			true,
		//			true,
		//			D3D12_CULL_MODE_BACK,
		//			D3D12_COMPARISON_FUNC_LESS_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{
		//				mainRTVFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		//		});
		//}
		//// Particle system buffer generation
		//{
		//	const uint64_t bufferGenerationShaderGuid = uint64_t::StringToGuid("shaders/particlesbuffergeneration.hlsl"); //shaders/particlesbuffergeneration.hlsl
		//	const uint64_t bufferGenerationPipelineGuid = uint64_t::Random();
		//	m_particleBufferGenerationComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//		bufferGenerationPipelineGuid,
		//		{
		//			bufferGenerationShaderGuid,
		//		});
		//}
		//// Fog post-effect
		//{
		//	const uint64_t fogPostEffectShaderGuid = uint64_t::StringToGuid("shaders/fogpostprocess.hlsl"); //shaders/fogpostprocess.hlsl
		//	const uint64_t fogPostEffectSharedMaterialGuid = uint64_t::Random();
		//	m_fogPostProcessSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		fogPostEffectSharedMaterialGuid,
		//		{
		//			fogPostEffectShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
		//			false,
		//			false,
		//			false,
		//			D3D12_CULL_MODE_NONE,
		//			D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{
		//				mainRTVFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}
		//// SSLR post-effect
		//{
		//	const uint64_t sslrPostEffectShaderGuid = uint64_t::StringToGuid("shaders/sslrpostprocess.hlsl"); //shaders/sslrpostprocess.hlsl
		//	const uint64_t sslrPostEffectSharedMaterialGuid = uint64_t::Random();
		//	m_sslrPostProcessSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		sslrPostEffectSharedMaterialGuid,
		//		{
		//			sslrPostEffectShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
		//			false,
		//			false,
		//			false,
		//			D3D12_CULL_MODE_NONE,
		//			D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{
		//				mainRTVFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}
		//// SSAO
		//{
		//	// SSAO Generation
		//	{
		//		const uint64_t ssaoPostEffectShaderGuid = uint64_t::StringToGuid("shaders/ssaopostprocess.hlsl"); //shaders/ssaopostprocess.hlsl
		//		const uint64_t ssaoPostEffectSharedMaterialGuid = uint64_t::Random();
		//		m_ssaoPostProcessSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//			ssaoPostEffectSharedMaterialGuid,
		//			{
		//				ssaoPostEffectShaderGuid,
		//				JoyShaderTypeVertex | JoyShaderTypePixel,
		//				false,
		//				false,
		//				false,
		//				D3D12_CULL_MODE_NONE,
		//				D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//				{
		//					ssaoFormat
		//				},
		//				mainDSVFormat,
		//				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//			});
		//	}
		//	// SSAO Blur pass
		//	{
		//		const uint64_t ssaoBlurShaderGuid = uint64_t::StringToGuid("shaders/ssaoblur.hlsl"); //shaders/ssaoblur.hlsl
		//		const uint64_t ssaoBlurSharedMaterialGuid = uint64_t::Random();
		//		m_ssaoBlurSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//			ssaoBlurSharedMaterialGuid,
		//			{
		//				ssaoBlurShaderGuid,
		//				JoyShaderTypeVertex | JoyShaderTypePixel,
		//				false,
		//				false,
		//				false,
		//				D3D12_CULL_MODE_NONE,
		//				D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//				{
		//					ssaoFormat
		//				},
		//				mainDSVFormat,
		//				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//			});
		//	}
		//	// SSAO append
		//	{
		//		const uint64_t ssaoAppendShaderGuid = uint64_t::StringToGuid("shaders/ssaoappend.hlsl"); //shaders/ssaoappend.hlsl
		//		const uint64_t ssaoAppendSharedMaterialGuid = uint64_t::Random();
		//		m_ssaoAppendSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//			ssaoAppendSharedMaterialGuid,
		//			{
		//				ssaoAppendShaderGuid,
		//				JoyShaderTypeVertex | JoyShaderTypePixel,
		//				false,
		//				false,
		//				false,
		//				D3D12_CULL_MODE_NONE,
		//				D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		//				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//				{
		//					mainRTVFormat
		//				},
		//				mainDSVFormat,
		//				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//			});
		//	}
		//}
		//// Bloom
		//{
		//	// Bright pass
		//	{
		//		const uint64_t bloomBrightPassShaderGuid = uint64_t::StringToGuid("shaders/bloomBrightPass.hlsl"); //shaders/bloomBrightPass.hlsl
		//		const uint64_t bloomBrightPassPipelineGuid = uint64_t::Random();
		//		m_bloomBrightPassComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			bloomBrightPassPipelineGuid,
		//			{
		//				bloomBrightPassShaderGuid,
		//			});
		//	}
		//	// Vertical filter
		//	{
		//		const uint64_t verticalFilterShaderGuid = uint64_t::StringToGuid("shaders/bloomVerticalFilter.hlsl"); //shaders/bloomVerticalFilter.hlsl
		//		const uint64_t verticalFilterPipelineGuid = uint64_t::Random();
		//		m_bloomVerticalFilterComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			verticalFilterPipelineGuid,
		//			{
		//				verticalFilterShaderGuid,
		//			});
		//	}
		//	// Horizontal filter
		//	{
		//		const uint64_t horizontalFilterShaderGuid = uint64_t::StringToGuid("shaders/bloomHorizontalFilter.hlsl"); //shaders/bloomHorizontalFilter.hlsl
		//		const uint64_t horizontalFilterPipelineGuid = uint64_t::Random();
		//		m_bloomHorizontalFilterComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			horizontalFilterPipelineGuid,
		//			{
		//				horizontalFilterShaderGuid,
		//			});
		//	}
		//}
	}

	void EngineDataProvider::SetMaterialData(uint32_t index, uint32_t diffuseTextureIndex) const
	{
		StandardMaterialData* dataPtr = m_materials->GetPtr();
		dataPtr->data[index].diffuseTextureIndex = diffuseTextureIndex;
	}
}
