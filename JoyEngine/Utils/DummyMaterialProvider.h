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
		void CreateSampleMaterial(const std::string& materialName, GUID textureGuid);

		[[nodiscard]] GUID GetMaterialGuid(const std::string& materialName) const noexcept
		{
			ASSERT(m_sampleMaterials.find(materialName) != m_sampleMaterials.end());

			return m_sampleMaterials.find(materialName)->second.materialHandle->GetGuid();
		}
		[[nodiscard]] SharedMaterial* GetGBufferSharedMaterial() const noexcept { return m_gbufferWriteSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetLightProcessingSharedMaterial() const noexcept { return m_lightProcessingSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetDirectionLightProcessingSharedMaterial() const noexcept { return m_directionLightProcessingSharedMaterial; }
	private:
		ResourceHandle<Texture> m_skyboxTextureHandle;

		ResourceHandle<SharedMaterial> m_gbufferWriteSharedMaterial;
		ResourceHandle<Shader> m_gbufferWriteShader;

		ResourceHandle<SharedMaterial> m_directionLightProcessingSharedMaterial;
		ResourceHandle<Shader> m_directionLightProcessingShader;

		ResourceHandle<SharedMaterial> m_lightProcessingSharedMaterial;
		ResourceHandle<Shader> m_lightProcessingShader;


		struct MaterialData
		{
			ResourceHandle<Texture> textureHandle;
			ResourceHandle<Material> materialHandle;
		};

		ResourceHandle<SharedMaterial> m_sharedMaterialHandle;
		ResourceHandle<Shader> m_shaderHandle;
		std::map<std::string, MaterialData> m_sampleMaterials;
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
