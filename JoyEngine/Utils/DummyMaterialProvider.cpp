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
		{
			const GUID shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
			const GUID sharedMaterialGuid = GUID::Random();
			const GUID materialGuid = GUID::Random();
			const GUID texture1Guid = GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0");



			CD3DX12_DESCRIPTOR_RANGE1 ranges[4];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);


			std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(5);
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[2].InitAsConstants(sizeof(MVP) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			rootParameters[3].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[4].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_PIXEL);


			m_sharedMaterialHandle = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
				sharedMaterialGuid,
				{
					shaderGuid,
					true,
					true,
					true,
					true,
					rootParameters,
					1
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

		{
			const GUID gbufferWriteShaderGuid = GUID::StringToGuid("48ffacc9-5c00-4058-b359-cf72189896ac");
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
					true,
					rootParameters,
					2
				});
		}
	}
}
