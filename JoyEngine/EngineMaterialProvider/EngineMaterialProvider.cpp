#include "EngineMaterialProvider.h"

#include <d3d12.h>

#include "d3dx12.h"
#include "DescriptorManager/DescriptorManager.h"
#include "GraphicsManager/GraphicsManager.h"
#include "RenderManager/RenderManager.h"
#include "Utils/TimeCounter.h"

using Microsoft::WRL::ComPtr;

#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	IMPLEMENT_SINGLETON(EngineMaterialProvider)

	void EngineMaterialProvider::Init()
	{
		TIME_PERF("EngineMaterialProvider init")

		EngineSamplersProvider::InitSamplers();
		DXGI_FORMAT mainRTVFormat = RenderManager::Get()->GetHdrRTVFormat();
		DXGI_FORMAT swapchainLdrFormat = RenderManager::Get()->GetLdrRTVFormat();
		DXGI_FORMAT mainGBufferFormat = RenderManager::Get()->GetGBufferFormat();
		DXGI_FORMAT mainDSVFormat = RenderManager::Get()->GetDepthFormat();
		DXGI_FORMAT ssaoFormat = RenderManager::Get()->GetSSAOFormat();


		// Standard shared material
		{
			const GUID standardSharedMaterialGuid = GUID::StringToGuid("b6316780-7043-4ca5-96da-c8bb84042b78");
			m_standardSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial>(standardSharedMaterialGuid);
		}

		// Mip map generation
		{
			const GUID mipMapGenerationShaderGuid = GUID::StringToGuid("3fb4d89b-ceab-46c3-b34f-d41a49e072cf"); //shaders/generateMipMaps.hlsl
			const GUID mipMapGenerationPipelineGuid = GUID::Random();

			m_generateMipsComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
				mipMapGenerationPipelineGuid,
				{
					mipMapGenerationShaderGuid,
				});
		}

		// Gizmo Axis draw 
		{
			const GUID gizmoAxisDrawerShaderGuid = GUID::StringToGuid("c8478fd3-33c3-4f3f-9e8a-56a9cfa846ce"); // shaders/gizmoAxisDrawer.hlsl
			const GUID gizmoAxisDrawerSharedMaterialGuid = GUID::Random();

			m_gizmoAxisDrawerSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
				gizmoAxisDrawerSharedMaterialGuid,
				{
					gizmoAxisDrawerShaderGuid,
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


			for (uint32_t i = 1; i < READONLY_TEXTURES_COUNT; i++)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
				DescriptorManager::Get()->GetDescriptorHandleAtIndex(DescriptorHeapType::READONLY_TEXTURES, i, cpuHandle, gpuHandle);

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


		//// GBuffer write shader
		//{
		//	const GUID gbufferWriteShaderGuid = GUID::StringToGuid("48ffacc9-5c00-4058-b359-cf72189896ac"); //shaders/gbufferwrite.hlsl
		//	const GUID gbufferWriteSharedMaterialGuid = GUID::Random();

		//	m_gbufferWriteSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		gbufferWriteSharedMaterialGuid,
		//		{
		//			gbufferWriteShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
		//			true,
		//			true,
		//			true,
		//			D3D12_CULL_MODE_BACK,
		//			D3D12_COMPARISON_FUNC_LESS_EQUAL,
		//			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		//			{
		//				mainGBufferFormat,
		//				mainGBufferFormat,
		//				mainGBufferFormat
		//			},
		//			mainDSVFormat,
		//			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		//		});
		//}


		//// Shadow map for spot light creation 
		//{
		//	const GUID shadowProcessingShaderGuid = GUID::StringToGuid("9ee0a40a-c055-4b2c-93db-bc19def8e8cc"); //shaders/shadowprocessing.hlsl
		//	const GUID shadowProcessingSharedMaterialGuid = GUID::Random();

		//	m_shadowProcessingSharedMaterial = ResourceManager::Get()->LoadResource<SharedMaterial, GraphicsPipelineArgs>(
		//		shadowProcessingSharedMaterialGuid,
		//		{
		//			shadowProcessingShaderGuid,
		//			JoyShaderTypeVertex | JoyShaderTypePixel,
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

		//// Shadow map for point map creation 
		//{
		//	const GUID shadowPointProcessingShaderGuid = GUID::StringToGuid("9d678808-8c11-4ff3-9ee1-dd1b7fc5f691"); //shaders/shadowpointprocessing.hlsl
		//	const GUID shadowPointProcessingSharedMaterialGuid = GUID::Random();

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
		//	const GUID directionLightProcessingShaderGuid = GUID::StringToGuid("1c6cb88f-f3ef-4797-9d65-44682ca7baba"); //shaders/directionlightprocessing.hlsl
		//	const GUID directionLightProcessingSharedMaterialGuid = GUID::Random();

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
		//	const GUID lightProcessingShaderGuid = GUID::StringToGuid("f9da7adf-4ebb-4601-8437-a19c07e8471a"); //shaders/lightprocessing.hlsl
		//	const GUID lightProcessingSharedMaterialGuid = GUID::Random();

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
		//		const GUID dynamicCubemapShaderGUID = GUID::StringToGuid("4a8ea369-904f-4d9a-9061-b4eedacc3918"); // shaders/dynamiccubemapreflections.hlsl
		//		const GUID sharedMaterialGuid = GUID::Random();

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
		//		const GUID cubemapConvolutionShaderGuid = GUID::StringToGuid("613710d9-304e-453b-a655-b1c842904f4c"); //shaders/dynamiccubemapconvolition.hlsl
		//		const GUID cubemapConvolutionSharedMaterialGuid = GUID::Random();

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
		//	const GUID shaderGuid = GUID::StringToGuid("a36fff56-b183-418a-9bd1-31cffd247e37"); // shaders/particles.hlsl
		//	const GUID sharedMaterialGuid = GUID::Random();

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
		//	const GUID bufferGenerationShaderGuid = GUID::StringToGuid("38d4f011-405a-4602-8f8e-79b4888d26b6"); //shaders/particlesbuffergeneration.hlsl
		//	const GUID bufferGenerationPipelineGuid = GUID::Random();

		//	m_particleBufferGenerationComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//		bufferGenerationPipelineGuid,
		//		{
		//			bufferGenerationShaderGuid,
		//		});
		//}

		//// Fog post-effect
		//{
		//	const GUID fogPostEffectShaderGuid = GUID::StringToGuid("5e897d4b-2ed5-4176-890b-2f17e52cb836"); //shaders/fogpostprocess.hlsl
		//	const GUID fogPostEffectSharedMaterialGuid = GUID::Random();

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
		//	const GUID sslrPostEffectShaderGuid = GUID::StringToGuid("70d5a60a-615a-4344-977f-d2ad9d37b0cf"); //shaders/sslrpostprocess.hlsl
		//	const GUID sslrPostEffectSharedMaterialGuid = GUID::Random();

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
		//		const GUID ssaoPostEffectShaderGuid = GUID::StringToGuid("3cf96278-de88-4614-bb23-5cfa2b54e41a"); //shaders/ssaopostprocess.hlsl
		//		const GUID ssaoPostEffectSharedMaterialGuid = GUID::Random();

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
		//		const GUID ssaoBlurShaderGuid = GUID::StringToGuid("81312dae-c920-4cd6-bd08-87d1a2a3b6f4"); //shaders/ssaoblur.hlsl
		//		const GUID ssaoBlurSharedMaterialGuid = GUID::Random();

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
		//		const GUID ssaoAppendShaderGuid = GUID::StringToGuid("c7986e89-5d7e-4348-903e-761f007c3f12"); //shaders/ssaoappend.hlsl
		//		const GUID ssaoAppendSharedMaterialGuid = GUID::Random();

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
		//		const GUID bloomBrightPassShaderGuid = GUID::StringToGuid("110f9d5d-008a-4f07-8a7c-e0a208353bf1"); //shaders/bloomBrightPass.hlsl
		//		const GUID bloomBrightPassPipelineGuid = GUID::Random();

		//		m_bloomBrightPassComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			bloomBrightPassPipelineGuid,
		//			{
		//				bloomBrightPassShaderGuid,
		//			});
		//	}
		//	// Vertical filter
		//	{
		//		const GUID verticalFilterShaderGuid = GUID::StringToGuid("79791a5c-5374-4e44-9ad0-f2cf57e7c8e6"); //shaders/bloomVerticalFilter.hlsl
		//		const GUID verticalFilterPipelineGuid = GUID::Random();

		//		m_bloomVerticalFilterComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			verticalFilterPipelineGuid,
		//			{
		//				verticalFilterShaderGuid,
		//			});
		//	}
		//	// Horizontal filter
		//	{
		//		const GUID horizontalFilterShaderGuid = GUID::StringToGuid("fcdfbb5b-72ea-4b7b-b432-3f807ffc576d"); //shaders/bloomHorizontalFilter.hlsl
		//		const GUID horizontalFilterPipelineGuid = GUID::Random();

		//		m_bloomHorizontalFilterComputePipeline = ResourceManager::Get()->LoadResource<ComputePipeline, ComputePipelineArgs>(
		//			horizontalFilterPipelineGuid,
		//			{
		//				horizontalFilterShaderGuid,
		//			});
		//	}
		//}
	}

	void EngineMaterialProvider::SetMaterialData(uint32_t index, uint32_t diffuseTextureIndex) const
	{
		m_materials->Lock();

		StandardMaterialData* dataPtr = m_materials->GetPtr();
		dataPtr->data[index].diffuseTextureIndex = diffuseTextureIndex;

		m_materials->Unlock();
	}
}
