﻿#include "DummyMaterialProvider.h"

#include <d3d12.h>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

#include "JoyContext.h"

#include "ResourceManager/ResourceManager.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/GUID.h"
#include "Utils/Assert.h"

namespace JoyEngine
{
	void DummyMaterialProvider::Init()
	{
		GUID shaderGuid = GUID::StringToGuid("183d6cfe-ca85-4e0b-ab36-7b1ca0f99d34");
		GUID sharedMaterialGuid = GUID::Random();
		GUID materialGuid = GUID::Random();
		GUID texture1Guid = GUID::StringToGuid("1d451f58-3f84-4b2b-8c6f-fe8e2821d7f0");

		SharedMaterial* sharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
			sharedMaterialGuid,
			{
				shaderGuid,
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

		m_material = JoyContext::Resource->LoadResource<Material, MaterialData>(
			materialGuid,
			{
				sharedMaterialGuid,
				material1RootParams,
				{texture1->GetImageViewHeap(),texture1->GetSampleHeap()}
			});
	}
}
