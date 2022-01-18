#include "DummyMaterialProvider.h"

#include <d3d12.h>

#include "d3dx12.h"

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
		m_shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
		m_sharedMaterialGuid = GUID::Random();
		m_materialGuid = GUID::Random();
		GUID texture1Guid = GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0");

		JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
			m_sharedMaterialGuid,
			{
				m_shaderGuid,
				true,
				true,
				true,
				true
			});
		Texture* texture1 = JoyContext::Resource->LoadResource<Texture>(texture1Guid);

		std::map<uint32_t, ID3D12DescriptorHeap*> material1RootParams = {
			{0, texture1->GetImageViewHeap()},
			{1, texture1->GetSampleHeap()}
		};

		JoyContext::Resource->LoadResource<Material, MaterialData>(
			m_materialGuid,
			{
				m_sharedMaterialGuid,
				material1RootParams,
				{texture1->GetImageViewHeap(),texture1->GetSampleHeap()}
			});
	}

	DummyMaterialProvider::~DummyMaterialProvider()
	{
		JoyContext::Resource->UnloadResource(m_sharedMaterialGuid);
		JoyContext::Resource->UnloadResource(m_materialGuid);
	}
}
