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

		[[nodiscard]] GUID GetMaterialGuid(const std::string& materialName) const noexcept
		{
			ASSERT(m_sampleMaterials.find(materialName) != m_sampleMaterials.end());

			return m_sampleMaterials.find(materialName)->second->GetGuid();
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

		[[nodiscard]] ComputePipeline* GetHdrDownscaleFirstPassComputePipeline() const noexcept { return m_hdrDownscaleFirstPassComputePipeline; }
		[[nodiscard]] ComputePipeline* GetHdrDownscaleSecondPassComputePipeline() const noexcept { return m_hdrDownscaleSecondPassComputePipeline; }
		[[nodiscard]] SharedMaterial* GetHdrToLdrTransitionSharedMaterial() const noexcept { return m_hdrToLdrTransitionSharedMaterial; }

		[[nodiscard]] ComputePipeline* GetBloomBrightPassComputePipeline() const noexcept { return m_bloomBrightPassComputePipeline; }
		[[nodiscard]] ComputePipeline* GetBloomVerticalFilterComputePipeline() const noexcept { return m_bloomVerticalFilterComputePipeline; }
		[[nodiscard]] ComputePipeline* GetBloomHorizontalFilterComputePipeline() const noexcept { return m_bloomHorizontalFilterComputePipeline; }

		[[nodiscard]] SharedMaterial* GetSsaoPostProcessSharedMaterial() const noexcept { return m_ssaoPostProcessSharedMaterial; }
		[[nodiscard]] SharedMaterial* GetSsaoBlurSharedMaterial() const noexcept { return m_ssaoBlurSharedMaterial; }

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

		ResourceHandle<ComputePipeline> m_bloomBrightPassComputePipeline;
		ResourceHandle<ComputePipeline> m_bloomVerticalFilterComputePipeline;
		ResourceHandle<ComputePipeline> m_bloomHorizontalFilterComputePipeline;


		ResourceHandle<SharedMaterial> m_sampleSharedMaterialHandle;
		ResourceHandle<SharedMaterial> m_dynamicCubemapReflectionsSharedMaterialHandle;
		std::map<std::string, ResourceHandle<Material>> m_sampleMaterials;

		ResourceHandle<SharedMaterial> m_ssaoPostProcessSharedMaterial;
		ResourceHandle<SharedMaterial> m_ssaoBlurSharedMaterial;

	private:
		void CreatePBRMaterial(
			const std::string& materialName,
			const GUID diffuseTextureGuid,
			const GUID normalTextureGuid,
			const GUID specularTextureGuid,
			const GUID roughnessTextureGuid,
			const GUID environmentTextureGuid,
			const GUID sharedMaterialGuid);

		void CreateSampleMaterial(
			const std::string& materialName,
			const GUID textureGuid,
			const GUID sharedMaterialGuid);
	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
