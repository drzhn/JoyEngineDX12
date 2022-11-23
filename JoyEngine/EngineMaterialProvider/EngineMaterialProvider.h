#ifndef DUMMY_MATERIAL_PROVIDER_H
#define DUMMY_MATERIAL_PROVIDER_H

#include "Common/Singleton.h"

#include "CommonEngineStructs.h"
#include "ResourceManager/ConstantCpuBuffer.h"
#include "ResourceManager/DynamicCpuBuffer.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/ResourceView.h"

namespace JoyEngine
{
	class EngineMaterialProvider : public Singleton<EngineMaterialProvider>
	{
	public:
		EngineMaterialProvider() = default;
		~EngineMaterialProvider() = default;
		void Init();

		[[nodiscard]] ResourceHandle<SharedMaterial> GetStandardSharedMaterial() const noexcept { return m_standardSharedMaterial; }
		[[nodiscard]] ResourceHandle<ComputePipeline> GetMipsGenerationComputePipeline() const noexcept { return m_generateMipsComputePipeline; }
		[[nodiscard]] ResourceHandle<SharedMaterial> GetGizmoAxisDrawerSharedMaterial() const noexcept { return m_gizmoAxisDrawerSharedMaterial; }
		[[nodiscard]] ResourceView* GetNullTextureView() const noexcept { return m_nullTextureView.get(); }
		[[nodiscard]] ResourceView* GetMaterialsDataView() const noexcept { return m_materials->GetView(); }
		[[nodiscard]] ResourceView* GetObjectMatricesDataView(uint32_t index) const noexcept { return m_objectMatrices->GetView(index); }

		[[nodiscard]] DynamicCpuBuffer<ObjectMatricesData>* GetObjectMatricesDataBuffer() const noexcept { return m_objectMatrices.get(); }

		void SetMaterialData(uint32_t index, uint32_t diffuseTextureIndex) const;

		//[[nodiscard]] ResourceHandle<SharedMaterial> GetGBufferSharedMaterial() const noexcept { return m_gbufferWriteSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetLightProcessingSharedMaterial() const noexcept { return m_lightProcessingSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetDirectionLightProcessingSharedMaterial() const noexcept { return m_directionLightProcessingSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetShadowProcessingSharedMaterial() const noexcept { return m_shadowProcessingSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetShadowPointProcessingSharedMaterial() const noexcept { return m_shadowPointProcessingSharedMaterial; }
		//[[nodiscard]] ResourceHandle<ComputePipeline> GetParticleBufferGenerationComputePipeline() const noexcept { return m_particleBufferGenerationComputePipeline; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetParticleSystemSharedMaterial() const noexcept { return m_particleSystemSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetFogPostProcessSharedMaterial() const noexcept { return m_fogPostProcessSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetSSLRPostProcessSharedMaterial() const noexcept { return m_sslrPostProcessSharedMaterial; }
		//[[nodiscard]] ResourceHandle<ComputePipeline> GetBloomBrightPassComputePipeline() const noexcept { return m_bloomBrightPassComputePipeline; }
		//[[nodiscard]] ResourceHandle<ComputePipeline> GetBloomVerticalFilterComputePipeline() const noexcept { return m_bloomVerticalFilterComputePipeline; }
		//[[nodiscard]] ResourceHandle<ComputePipeline> GetBloomHorizontalFilterComputePipeline() const noexcept { return m_bloomHorizontalFilterComputePipeline; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetSsaoPostProcessSharedMaterial() const noexcept { return m_ssaoPostProcessSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetSsaoBlurSharedMaterial() const noexcept { return m_ssaoBlurSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetSsaoAppendSharedMaterial() const noexcept { return m_ssaoAppendSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetCubemapConvolutionSharedMaterial() const noexcept { return m_cubemapConvolutionSharedMaterial; }

	private:
		ResourceHandle<SharedMaterial> m_standardSharedMaterial;

		ResourceHandle<ComputePipeline> m_generateMipsComputePipeline;

		ResourceHandle<SharedMaterial> m_gizmoAxisDrawerSharedMaterial;

		std::unique_ptr<ResourceView> m_nullTextureView;

		std::unique_ptr<ConstantCpuBuffer<StandardMaterialData>> m_materials;

		std::unique_ptr<DynamicCpuBuffer<ObjectMatricesData>> m_objectMatrices;

		//ResourceHandle<SharedMaterial> m_gbufferWriteSharedMaterial;

		//ResourceHandle<SharedMaterial> m_shadowProcessingSharedMaterial;

		//ResourceHandle<SharedMaterial> m_shadowPointProcessingSharedMaterial;

		//ResourceHandle<SharedMaterial> m_directionLightProcessingSharedMaterial;

		//ResourceHandle<SharedMaterial> m_lightProcessingSharedMaterial;

		//ResourceHandle<ComputePipeline> m_particleBufferGenerationComputePipeline;

		//ResourceHandle<SharedMaterial> m_particleSystemSharedMaterial;

		//ResourceHandle<SharedMaterial> m_fogPostProcessSharedMaterial;

		//ResourceHandle<ComputePipeline> m_bloomBrightPassComputePipeline;
		//ResourceHandle<ComputePipeline> m_bloomVerticalFilterComputePipeline;
		//ResourceHandle<ComputePipeline> m_bloomHorizontalFilterComputePipeline;


		//ResourceHandle<SharedMaterial> m_sampleSharedMaterialHandle;
		//std::map<std::string, ResourceHandle<Material>> m_sampleMaterials;

		//ResourceHandle<SharedMaterial> m_dynamicCubemapReflectionsSharedMaterialHandle;
		//ResourceHandle<SharedMaterial> m_cubemapConvolutionSharedMaterial;

		//ResourceHandle<SharedMaterial> m_ssaoPostProcessSharedMaterial;
		//ResourceHandle<SharedMaterial> m_ssaoBlurSharedMaterial;
		//ResourceHandle<SharedMaterial> m_ssaoAppendSharedMaterial;

		//ResourceHandle<SharedMaterial> m_sslrPostProcessSharedMaterial;

	};
}

#endif // DUMMY_MATERIAL_PROVIDER_H
