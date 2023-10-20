#ifndef ENGINE_DATA_PROVIDER_H
#define ENGINE_DATA_PROVIDER_H

#include "Common/Singleton.h"

#include "CommonEngineStructs.h"
#include "MeshContainer.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/ResourceView.h"
#include "ResourceManager/Buffers/ConstantCpuBuffer.h"
#include "ResourceManager/Buffers/DynamicCpuBuffer.h"

namespace JoyEngine
{
	class EngineDataProvider : public Singleton<EngineDataProvider>
	{
	public:
		EngineDataProvider() = default;
		~EngineDataProvider() = default;
		void Init();

		[[nodiscard]] ResourceHandle<SharedMaterial> GetStandardSharedMaterial() const noexcept { return m_standardSharedMaterial; }
		//[[nodiscard]] ResourceHandle<ComputePipeline> GetMipsGenerationComputePipeline() const noexcept { return m_generateMipsComputePipeline; }
		[[nodiscard]] ResourceHandle<SharedMaterial> GetGizmoAxisDrawerSharedMaterial() const noexcept { return m_gizmoAxisDrawerSharedMaterial; }
		[[nodiscard]] ResourceHandle<SharedMaterial> GetStandardPhongSharedMaterial() const noexcept { return m_standardPhongSharedMaterial; }
		[[nodiscard]] ResourceHandle<SharedMaterial> GetDeferredShadingProcessorSharedMaterial() const noexcept { return m_deferredShadingProcessorSharedMaterial; }
		[[nodiscard]] ResourceHandle<SharedMaterial> GetShadowProcessingSharedMaterial() const noexcept { return m_shadowProcessingSharedMaterial; }

		[[nodiscard]] ResourceView* GetNullTextureView() const noexcept { return m_nullTextureView.get(); }
		[[nodiscard]] ResourceView* GetMaterialsDataView() const noexcept { return m_materials->GetView(); }
		[[nodiscard]] ResourceView* GetEngineDataView(uint32_t index) const noexcept { return m_engineDataBuffer->GetView(index); }

		[[nodiscard]] MeshContainer* GetMeshContainer() const { return m_meshContainer.get(); }

		[[nodiscard]] DynamicCpuBuffer<EngineData>* GetEngineDataBuffer() const noexcept { return m_engineDataBuffer.get(); }

		void SetMaterialData(const uint32_t materialIndex, const size_t fieldOffset, const void* valuePtr, const size_t valueSize) const;

		//[[nodiscard]] ResourceHandle<SharedMaterial> GetLightProcessingSharedMaterial() const noexcept { return m_lightProcessingSharedMaterial; }
		//[[nodiscard]] ResourceHandle<SharedMaterial> GetDirectionLightProcessingSharedMaterial() const noexcept { return m_directionLightProcessingSharedMaterial; }
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

		ResourceHandle<SharedMaterial> m_gizmoAxisDrawerSharedMaterial;

		ResourceHandle<SharedMaterial> m_standardPhongSharedMaterial;

		ResourceHandle<SharedMaterial> m_deferredShadingProcessorSharedMaterial;

		ResourceHandle<SharedMaterial> m_shadowProcessingSharedMaterial;

		std::unique_ptr<ResourceView> m_nullTextureView;

		std::unique_ptr<ConstantCpuBuffer<StandardMaterialData>> m_materials;

		std::unique_ptr<DynamicCpuBuffer<EngineData>> m_engineDataBuffer;

		std::unique_ptr<MeshContainer> m_meshContainer;

		//ResourceHandle<ComputePipeline> m_generateMipsComputePipeline;

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

#endif // ENGINE_DATA_PROVIDER_H
