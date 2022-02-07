#include "DummyMaterialProvider.h"

#include <d3d12.h>

#include "d3dx12.h"
#include "RenderManager/JoyTypes.h"

using Microsoft::WRL::ComPtr;

#include "JoyContext.h"

#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	void DummyMaterialProvider::Init()
	{
		//GUID skyboxTextureGuid = GUID::StringToGuid("ab9f4108-d126-4390-8233-75ee3fed4584");
		//m_skyboxTextureHandle = JoyContext::Resource->LoadResource<Texture>(skyboxTextureGuid);
		// GBuffer write shader
		{
			const GUID gbufferWriteShaderGuid = GUID::StringToGuid("48ffacc9-5c00-4058-b359-cf72189896ac"); //shaders/gbufferwrite.hlsl
			const GUID gbufferWriteSharedMaterialGuid = GUID::Random();

			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(1);
			rootParameters[0].InitAsConstants(sizeof(MVP) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			m_gbufferWriteSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				gbufferWriteSharedMaterialGuid,
				{
					gbufferWriteShaderGuid,
					true,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rootParameters,
					{
						DXGI_FORMAT_R16G16B16A16_FLOAT,
						DXGI_FORMAT_R16G16B16A16_FLOAT
					},
					DXGI_FORMAT_D32_FLOAT
				});
		}


		// Shadow map creation 
		{
			const GUID shadowProcessingShaderGuid = GUID::StringToGuid("9ee0a40a-c055-4b2c-93db-bc19def8e8cc"); //shaders/shadowprocessing.hlsl
			const GUID shadowProcessingSharedMaterialGuid = GUID::Random();

			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(1);
			rootParameters[0].InitAsConstants(sizeof(MVP) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

			m_shadowProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				shadowProcessingSharedMaterialGuid,
				{
					shadowProcessingShaderGuid,
					true,
					true,
					true,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rootParameters,
					{}, // no rtv, only depth
					DXGI_FORMAT_D32_FLOAT
				});
		}

		// Direction light processing
		{
			const GUID directionLightProcessingShaderGuid = GUID::StringToGuid("1c6cb88f-f3ef-4797-9d65-44682ca7baba"); //shaders/directionlightprocessing.hlsl
			const GUID directionLightProcessingSharedMaterialGuid = GUID::Random();

			CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(2);
			rootParameters[0].InitAsConstants(sizeof(LightData) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

			m_directionLightProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				directionLightProcessingSharedMaterialGuid,
				{
					directionLightProcessingShaderGuid,
					false,
					false,
					false,
					D3D12_CULL_MODE_NONE,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rootParameters,
					{
						DXGI_FORMAT_R8G8B8A8_UNORM
					},
					DXGI_FORMAT_D32_FLOAT
				});
		}

		// Light processing
		{
			const GUID lightProcessingShaderGuid = GUID::StringToGuid("f9da7adf-4ebb-4601-8437-a19c07e8471a"); //shaders/lightprocessing.hlsl
			const GUID lightProcessingSharedMaterialGuid = GUID::Random();

			CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(3);
			rootParameters[0].InitAsConstants(sizeof(LightData) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

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
					true,
					true,
					false,
					D3D12_CULL_MODE_FRONT,
					D3D12_COMPARISON_FUNC_GREATER_EQUAL,
					blendDesc,
					rootParameters,
					{
						DXGI_FORMAT_R8G8B8A8_UNORM
					},
					DXGI_FORMAT_D32_FLOAT
				});
		}


		// Sample material
		{
			const GUID shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
			const GUID sharedMaterialGuid = GUID::Random();


			CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);


			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(4);
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[2].InitAsConstants(sizeof(MVP) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[3].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);


			m_sharedMaterialHandle = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sharedMaterialGuid,
				{
					shaderGuid,
					true,
					true,
					false,
					D3D12_CULL_MODE_BACK,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					CD3DX12_BLEND_DESC(D3D12_DEFAULT),
					rootParameters,
					{
						DXGI_FORMAT_R8G8B8A8_UNORM
					},
					DXGI_FORMAT_D32_FLOAT
				});

			CreateSampleMaterial("material_1", GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0")); // viking_room.png
			CreateSampleMaterial("material_2", GUID::StringToGuid("e8448435-7baf-4e40-ac72-b99e49284929")); // textures/wood.png
			CreateSampleMaterial("material_3", GUID::StringToGuid("7e50aa82-5696-428c-a088-538fb78c0ee6")); // textures/Chopping-Board.jpg
		}
	}

	void DummyMaterialProvider::CreateSampleMaterial(const std::string& materialName, const GUID textureGuid)
	{
		const GUID materialGuid = GUID::Random();
		ResourceHandle<Texture> texture = ResourceHandle(JoyContext::Resource->LoadResource<Texture>(textureGuid));

		const std::map<uint32_t, ID3D12DescriptorHeap*> materialRootParams = {
			{0, texture->GetResourceView()->GetHeap()},
			{1, texture->GetSampleView()->GetHeap()}
		};
		ResourceHandle<Material> material = ResourceHandle(JoyContext::Resource->LoadResource<Material, MaterialArgs>(
			materialGuid,
			{
				m_sharedMaterialHandle->GetGuid(),
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
