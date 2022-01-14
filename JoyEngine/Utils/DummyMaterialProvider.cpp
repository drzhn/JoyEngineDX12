#include "DummyMaterialProvider.h"

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
		SharedMaterial* sharedMaterial = JoyContext::Resource->LoadResource<SharedMaterial, SharedMaterialArgs>(
			sharedMaterialGuid,
			{
				shaderGuid,
				true,
				true,
				true,
				true
			});
	}
}
