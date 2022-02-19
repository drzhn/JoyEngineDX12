#ifndef DUMMY_MATERIAL_PROVIDER_H
#define DUMMY_MATERIAL_PROVIDER_H

#include "ResourceManager/Material.h"
#include "ResourceManager/SharedMaterial.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	struct RootParams
	{
		// I'm not sorry
		std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;
		std::vector<CD3DX12_ROOT_PARAMETER1> params;

		void CreateDescriptorTable(
			D3D12_DESCRIPTOR_RANGE_TYPE type,
			uint32_t shaderRegister,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL,
			D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

		void CreateConstants(
			uint32_t number,
			uint32_t shaderRegister,
			D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	};

	class DummyMaterialProvider
	{
	public:
		DummyMaterialProvider() = default;
		~DummyMaterialProvider() = default;
		void Init();
		void CreateSampleMaterial(const std::string& materialName, const GUID textureGuid, const GUID sharedMaterialGuid);

		[[nodiscard]] GUID GetMaterialGuid(const std::string& materialName) const noexcept
		{
			ASSERT(m_sampleMaterials.find(materialName) != m_sampleMaterials.end());

			return m_sampleMaterials.find(materialName)->second.materialHandle->GetGuid();
		}

		[[nodiscard]] ComputePipeline* GetMipsGenerationComputePipeline() const noexcept { return m_generateMipsComputePipeline; }
		[[nodiscard]] SharedMaterial* GetGBufferSharedMaterial() const noexcept { return m_gbufferWriteSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetLightProcessingSharedMaterial() const noexcept { return m_lightProcessingSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetDirectionLightProcessingSharedMaterial() const noexcept { return m_directionLightProcessingSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetShadowProcessingSharedMaterial() const noexcept { return m_shadowProcessingSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetShadowPointProcessingSharedMaterial() const noexcept { return m_shadowPointProcessingSharedMaterial; }
		[[nodiscard]] ComputePipeline* GetParticleBufferGenerationComputePipeline() const noexcept { return m_particleBufferGenerationComputePipeline; }
		[[nodiscard]] SharedMaterial* GetParticleSystemSharedMaterial() const noexcept { return m_particleSystemSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetFogPostProcessSharedMaterial() const noexcept { return m_fogPostProcessSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetHdrToLdrTransitionSharedMaterial() const noexcept { return m_hdrToLdrTransitionSharedMaterial; }
	private:
		//ResourceHandle<Texture> m_skyboxTextureHandle;

		ResourceHandle<ComputePipeline> m_generateMipsComputePipeline;

		ResourceHandle<SharedMaterial> m_gbufferWriteSharedMaterial;

		ResourceHandle<SharedMaterial> m_shadowProcessingSharedMaterial;

		ResourceHandle<SharedMaterial> m_shadowPointProcessingSharedMaterial;

		ResourceHandle<SharedMaterial> m_directionLightProcessingSharedMaterial;

		ResourceHandle<SharedMaterial> m_lightProcessingSharedMaterial;

		ResourceHandle<ComputePipeline> m_particleBufferGenerationComputePipeline;

		ResourceHandle<SharedMaterial> m_particleSystemSharedMaterial;

		ResourceHandle<SharedMaterial> m_fogPostProcessSharedMaterial;

		ResourceHandle<ComputePipeline> m_hdrDownscaleFirstPassComputePipeline;
		ResourceHandle<ComputePipeline> m_hdrDownscaleSecondPassComputePipeline;
		ResourceHandle<SharedMaterial> m_hdrToLdrTransitionSharedMaterial;


		struct MaterialData
		{
			ResourceHandle<Texture> textureHandle;
			ResourceHandle<Material> materialHandle;
		};

		ResourceHandle<SharedMaterial> m_sampleSharedMaterialHandle;
		ResourceHandle<SharedMaterial> m_dynamicCubemapReflectionsSharedMaterialHandle;
		std::map<std::string, MaterialData> m_sampleMaterials;
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
