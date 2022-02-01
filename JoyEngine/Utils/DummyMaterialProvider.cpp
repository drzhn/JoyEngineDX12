﻿#include "DummyMaterialProvider.h"

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
					}
				});
		}

		// Direction light processing
		{
			const GUID directionLightProcessingShaderGuid = GUID::StringToGuid("1c6cb88f-f3ef-4797-9d65-44682ca7baba"); //shaders/directionlightprocessing.hlsl
			const GUID directionLightProcessingSharedMaterialGuid = GUID::Random();

			//CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
			//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

			//std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(3);
			//rootParameters[0].InitAsConstants(sizeof(LightData) / 4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
			//rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			//rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

			//m_lightProcessingSharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
			//	directionLightProcessingSharedMaterialGuid,
			//	{
			//		directionLightProcessingShaderGuid,
			//		true,
			//		true,
			//		false,
			//		D3D12_CULL_MODE_FRONT,
			//		D3D12_COMPARISON_FUNC_GREATER_EQUAL,
			//		rootParameters,
			//		{
			//			DXGI_FORMAT_R8G8B8A8_UNORM
			//		}
			//	});
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
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			{
				blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
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
					}
				});
		}


		// Sample material
		{
			const GUID shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
			const GUID sharedMaterialGuid = GUID::Random();
			const GUID materialGuid = GUID::Random();
			const GUID texture1Guid = GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0");


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
					}
				});
			m_textureHandle = JoyContext::Resource->LoadResource<Texture>(texture1Guid);

			const std::map<uint32_t, ID3D12DescriptorHeap*> material1RootParams = {
				{0, m_textureHandle->GetResourceView()->GetHeap()},
				{1, m_textureHandle->GetSampleView()->GetHeap()}
			};

			m_materialHandle = JoyContext::Resource->LoadResource<Material, MaterialData>(
				materialGuid,
				{
					sharedMaterialGuid,
					material1RootParams,
				});
		}
	}
}
