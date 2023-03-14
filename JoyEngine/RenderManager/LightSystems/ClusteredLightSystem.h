#ifndef CLUSTERED_LIGHT_SYSTEM_H
#define CLUSTERED_LIGHT_SYSTEM_H

#include <set>

#include "CommonEngineStructs.h"
#include "ILightSystem.h"
#include "Components/Light.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"
#include "ResourceManager/Buffers/DynamicCpuBuffer.h"



namespace JoyEngine
{
	class Camera;
	class SharedMaterial;
	class ResourceView;

	class ClusteredLightSystem : public ILightSystem
	{
	public:
		explicit ClusteredLightSystem(uint32_t frameCount);

		void SetCamera(Camera* camera) { m_camera = camera; }

		void Update(const uint32_t frameIndex) override;

		// TODO I still do not have option how to get list of z-write geometry from scene,
		// So I just pass gbuffer shared material to this method bc it contains all of mesh renderers 
		void RenderDirectionalShadows(
			ID3D12GraphicsCommandList* commandList,
			uint32_t frameIndex,
			SharedMaterial* gBufferSharedMaterial);

		[[nodiscard]] ResourceView* GetDirectionalLightDataView(uint32_t frameIndex) const
		{
			return m_directionalLightDataBuffer->GetView(frameIndex);
		}

		[[nodiscard]] ResourceView* GetDirectionalShadowmapView() const
		{
			return m_directionalShadowmap->GetSRV();
		}

		DirectionalLightInfo& GetDirectionalLightData() override
		{
			return m_directionalLightData;
		}

		uint32_t RegisterDirectionalLight() override;
		void UnregisterDirectionalLight() override;

		uint32_t RegisterLight(LightBase* light) override;
		void UnregisterLight(LightBase* light) override;

		LightInfo& GetLightInfo(uint32_t lightIndex) override
		{
			return m_lightDataPool.GetElem(lightIndex);
		}

	private:
		const uint32_t m_frameCount;
		Camera* m_camera;

		DirectionalLightInfo m_directionalLightData;
		std::unique_ptr<DynamicCpuBuffer<DirectionalLightInfo>> m_directionalLightDataBuffer;
		std::unique_ptr<DepthTexture> m_directionalShadowmap;


		DynamicBufferPool<LightInfo, LightData, LIGHT_SIZE> m_lightDataPool;
		// TODO we store pointers to lights here for getting their positions during light clusterization
		// TODO make something smarter than this
		std::set<LightBase*> m_lights;
		std::array<uint32_t, NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z * LIGHTS_PER_CLUSTER> m_clusterLightIndices;
		DynamicCpuBuffer<ClusterEntryData> m_clusterEntryData;
		DynamicCpuBuffer<ClusterItemData> m_clusterItemData;
	};
}
#endif // CLUSTERED_LIGHT_SYSTEM_H
