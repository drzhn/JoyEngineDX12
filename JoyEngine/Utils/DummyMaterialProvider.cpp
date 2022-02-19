#include "DummyMaterialProvider.h"

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
	void RootParams::CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t shaderRegister, D3D12_SHADER_VISIBILITY visibility, D3D12_DESCRIPTOR_RANGE_FLAGS flags)
	{
		ranges.emplace_back();
		ranges.back().Init(type, 1, shaderRegister, 0, flags);
		params.emplace_back();
		params[params.size() - 1].InitAsDescriptorTable(1, &ranges.back(), visibility);
	}

	void RootParams::CreateConstants(uint32_t number, uint32_t shaderRegister, D3D12_SHADER_VISIBILITY visibility)
	{
		params.emplace_back();
		params[params.size() - 1].InitAsConstants(number, shaderRegister, 0, visibility);
	}

	void DummyMaterialProvider::Init()
	{
		Texture::InitSamplers();
		DXGI_FORMAT mainRTVFormat = RenderManager::GetHdrRTVFormat();
		DXGI_FORMAT swapchainLdrFormat = RenderManager::GetLdrRTVFormat();
		DXGI_FORMAT mainGBufferFormat = RenderManager::GetGBufferFormat();
		DXGI_FORMAT mainDSVFormat = RenderManager::GetDepthFormat();


		// Mip map generation
		{
			const GUID mipMapGenerationShaderGuid = GUID::StringToGuid("3fb4d89b-ceab-46c3-b34f-d41a49e072cf"); //shaders/generateMipMaps.hlsl
			const GUID mipMapGenerationPipelineGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 3);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0);
			rp.CreateConstants(sizeof(glm::vec2), 0);
			m_generateMipsComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
				mipMapGenerationPipelineGuid,
				{
					mipMapGenerationShaderGuid,
					rp.params
				});
		}

		// GBuffer write shader
		{
			const GUID gbufferWriteShaderGuid = GUID::StringToGuid("48ffacc9-5c00-4058-b359-cf72189896ac"); //shaders/gbufferwrite.hlsl
			const GUID gbufferWriteSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0, D3D12_SHADER_VISIBILITY_VERTEX);

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
					rp.params,
					{
						mainGBufferFormat,
						mainGBufferFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}


		// Shadow map for spot light creation 
		{
			const GUID shadowProcessingShaderGuid = GUID::StringToGuid("9ee0a40a-c055-4b2c-93db-bc19def8e8cc"); //shaders/shadowprocessing.hlsl
			const GUID shadowProcessingSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0, D3D12_SHADER_VISIBILITY_VERTEX);

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
					rp.params,
					{}, // no rtv, only depth
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}

		// Shadow map for point map creation 
		{
			const GUID shadowPointProcessingShaderGuid = GUID::StringToGuid("9d678808-8c11-4ff3-9ee1-dd1b7fc5f691"); //shaders/shadowpointprocessing.hlsl
			const GUID shadowPointProcessingSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);

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
					rp.params,
					{}, // no rtv, only depth
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}

		// Direction light processing
		{
			const GUID directionLightProcessingShaderGuid = GUID::StringToGuid("1c6cb88f-f3ef-4797-9d65-44682ca7baba"); //shaders/directionlightprocessing.hlsl
			const GUID directionLightProcessingSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(DirectionLightData) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}

		// Light processing
		{
			const GUID lightProcessingShaderGuid = GUID::StringToGuid("f9da7adf-4ebb-4601-8437-a19c07e8471a"); //shaders/lightprocessing.hlsl
			const GUID lightProcessingSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0, D3D12_SHADER_VISIBILITY_ALL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, D3D12_SHADER_VISIBILITY_PIXEL);

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
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}

		// Dynamic cubemap reflections
		{
			const GUID dynamicCubemapShaderGUID = GUID::StringToGuid("4a8ea369-904f-4d9a-9061-b4eedacc3918"); // shaders/dynamiccubemapreflections.hlsl
			const GUID sharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateConstants(sizeof(MVP) / 4, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);

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
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{
						{2, ModelViewProjection},
						{3, EnvironmentCubemap},
						{4, EngineData}
					}
				});

			CreateSampleMaterial("dynamic_reflections", GUID::StringToGuid("7e50aa82-5696-428c-a088-538fb78c0ee6"), sharedMaterialGuid); // textures/Chopping-Board.jpg
		}

		// Sample material
		{
			const GUID shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
			const GUID sharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateConstants(sizeof(MVP) / 4, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL);

			m_sampleSharedMaterialHandle = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sharedMaterialGuid,
				{
					shaderGuid,
					JoyShaderTypeVertex | JoyShaderTypePixel,
					true,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{
						{2, ModelViewProjection},
						{3, LightAttachment}
					}
				});

			CreateSampleMaterial("material_1", GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0"), sharedMaterialGuid); // viking_room.png
			CreateSampleMaterial("material_2", GUID::StringToGuid("e8448435-7baf-4e40-ac72-b99e49284929"), sharedMaterialGuid); // textures/wood.png
			CreateSampleMaterial("material_3", GUID::StringToGuid("7e50aa82-5696-428c-a088-538fb78c0ee6"), sharedMaterialGuid); // textures/Chopping-Board.jpg
		}

		// Particle system drawing
		{
			const GUID shaderGuid = GUID::StringToGuid("a36fff56-b183-418a-9bd1-31cffd247e37"); // shaders/particles.hlsl
			const GUID sharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateConstants(sizeof(MVP) / 4, 0);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);

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
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
					{}
				});
		}

		// Particle system buffer generation
		{
			const GUID bufferGenerationShaderGuid = GUID::StringToGuid("38d4f011-405a-4602-8f8e-79b4888d26b6"); //shaders/particlesbuffergeneration.hlsl
			const GUID bufferGenerationPipelineGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
			rp.CreateConstants(sizeof(float), 0);

			m_particleBufferGenerationComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
				bufferGenerationPipelineGuid,
				{
					bufferGenerationShaderGuid,
					rp.params
				});
		}

		// Fog post-effect
		{
			const GUID fogPostEffectShaderGuid = GUID::StringToGuid("5e897d4b-2ed5-4176-890b-2f17e52cb836"); //shaders/fogpostprocess.hlsl
			const GUID fogPostEffectSharedMaterialGuid = GUID::Random();

			RootParams rp;
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL);
			rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, D3D12_SHADER_VISIBILITY_ALL);

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
					rp.params,
					{
						mainRTVFormat
					},
					mainDSVFormat,
					D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
					{}
				});
		}

		// HDR
		{
			// Downscaling first pass
			{
				const GUID hdrDownscaleFirstPassShaderGuid = GUID::StringToGuid("e3e039f4-4f96-4e5b-b90b-1f46d460b724"); //shaders/hdrDownscaleFirstPass.hlsl
				const GUID hdrDownscaleFirstPassPipelineGuid = GUID::Random();

				RootParams rp;
				rp.CreateConstants(sizeof(HDRDownScaleConstants), 0);
				rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);
				rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);

				m_hdrDownscaleFirstPassComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					hdrDownscaleFirstPassPipelineGuid,
					{
						hdrDownscaleFirstPassShaderGuid,
						rp.params
					});
			}

			// Downscaling second pass
			{
				const GUID hdrDownscaleFirstPassShaderGuid = GUID::StringToGuid("c3a1592f-f12d-4c25-bcbb-1e6ace76b0fb"); //shaders/hdrDownscaleSecondPass.hlsl
				const GUID hdrDownscaleFirstPassPipelineGuid = GUID::Random();

				RootParams rp;
				rp.CreateConstants(sizeof(HDRDownScaleConstants), 0);
				rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);

				m_hdrDownscaleFirstPassComputePipeline = JoyContext::Resource->LoadResource<ComputePipeline, ComputePipelineArgs>(
					hdrDownscaleFirstPassPipelineGuid,
					{
						hdrDownscaleFirstPassShaderGuid,
						rp.params
					});
			}

			// HDR -> LDR transition
			{
				const GUID hdrToLdrTransitionShaderGuid = GUID::StringToGuid("aa366fc9-b8a7-4cca-b5d3-670216174566"); //shaders/hdrToLdrTransition.hlsl
				const GUID hdrToLdrTransitionSharedMaterialGuid = GUID::Random();

				RootParams rp;
				rp.CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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
						rp.params,
						{
							swapchainLdrFormat
						},
						mainDSVFormat,
						D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
						{}
					});
			}
		}
	}

	void DummyMaterialProvider::CreateSampleMaterial(const std::string& materialName, const GUID textureGuid, const GUID sharedMaterialGuid)
	{
		const GUID materialGuid = GUID::Random();
		ResourceHandle<Texture> texture = ResourceHandle(JoyContext::Resource->LoadResource<Texture>(textureGuid));

		// TODO make const std::map<uint32_t, ResourceView*>
		const std::map<uint32_t, ID3D12DescriptorHeap*> materialRootParams = {
			{0, texture->GetResourceView()->GetHeap()},
			{1, Texture::GetTextureSampler()->GetHeap()}
		};
		ResourceHandle<Material> material = ResourceHandle(JoyContext::Resource->LoadResource<Material, MaterialArgs>(
			materialGuid,
			{
				sharedMaterialGuid,
				materialRootParams,
			}));
		m_sampleMaterials.insert({
			materialName,
			{
				texture,
				material
			}
		});
	}
}
