#ifndef DUMMY_MATERIAL_PROVIDER_H
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
		[[nodiscard]] SharedMaterial* GetGBufferSharedMaterial() const noexcept { return m_gbufferWriteSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetLightProcessingSharedMaterial() const noexcept { return m_lightProcessingSharedMaterial; }
	private:
		ResourceHandle<SharedMaterial> m_gbufferWriteSharedMaterial;
		ResourceHandle<Shader> m_gbufferWriteShader;

		ResourceHandle<SharedMaterial> m_lightProcessingSharedMaterial;
		ResourceHandle<Shader> m_lightProcessingShader;

		ResourceHandle<Texture> m_textureHandle;
		ResourceHandle<Material> m_materialHandle;
		ResourceHandle<SharedMaterial> m_sharedMaterialHandle;
		ResourceHandle<Shader> m_shaderHandle;

	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
