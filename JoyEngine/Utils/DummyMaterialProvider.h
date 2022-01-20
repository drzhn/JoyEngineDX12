﻿#ifndef DUMMY_MATERIAL_PROVIDER_H
#define DUMMY_MATERIAL_PROVIDER_H

#include "ResourceManager/Material.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class DummyMaterialProvider
	{
	public:
		DummyMaterialProvider() = default;
		~DummyMaterialProvider() = default;
		void Init();

		[[nodiscard]] GUID GetMaterialGuid() const noexcept { return m_materialHandle->GetGuid(); }
		[[nodiscard]] SharedMaterial* GetGBufferSharedMaterialGuid() const noexcept { return m_gbufferWriteSharedMaterialGuid; }
	private:
		ResourceHandle<Texture> m_textureHandle;
		ResourceHandle<Material> m_materialHandle;
		ResourceHandle<SharedMaterial> m_sharedMaterialHandle;
		ResourceHandle<Shader> m_shaderHandle;

		ResourceHandle<SharedMaterial> m_gbufferWriteSharedMaterialGuid;
		ResourceHandle<Shader> m_gbufferWriteShaderGuid;
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
