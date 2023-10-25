#ifndef CLUSTERED_LIGHT_SYSTEM_H
#define CLUSTERED_LIGHT_SYSTEM_H

#include <map>

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
			SharedMaterial* gBufferSharedMaterial) const;

		DirectionalLightInfo& GetDirectionalLightData() override
		{
			return m_directionalLightData;
		}

		uint32_t RegisterDirectionalLight() override;
		void UnregisterDirectionalLight() override;

		uint32_t RegisterLight(LightBase* light) override;
		void UnregisterLight(LightBase* light) override;

		LightInfo& GetLightInfo(const uint32_t frameIndex, const uint32_t lightIndex) override
		{
			return m_lightDataPool.GetValue(lightIndex);
		}

		[[nodiscard]] ResourceView* GetDirectionalLightDataView(const uint32_t frameIndex) const
		{
			return m_directionalLightDataBuffer.GetView(frameIndex);
		}

		[[nodiscard]] ResourceView* GetDirectionalShadowmapView() const
		{
			return m_directionalShadowmap->GetSRV();
		}

		[[nodiscard]] ResourceView* GetClusterEntryDataView(const uint32_t frameIndex) const
		{
			return m_clusterEntryData.GetView(frameIndex);
		}

		[[nodiscard]] ResourceView* GetClusterItemDataView(const uint32_t frameIndex) const
		{
			return m_clusterItemData.GetView(frameIndex);
		}

		[[nodiscard]] ResourceView* GetLightDataView(const uint32_t frameIndex)
		{
			return m_lightDataPool.GetDynamicBuffer().GetView(frameIndex);
		}

	private:
		const uint32_t m_frameCount;
		Camera* m_camera;

		DirectionalLightInfo m_directionalLightData;
		DynamicCpuBuffer<DirectionalLightInfo> m_directionalLightDataBuffer;
		std::unique_ptr<DepthTexture> m_directionalShadowmap;


		DynamicBufferPool<LightInfo, LIGHT_SIZE> m_lightDataPool;
		// TODO we store pointers to lights here for getting their positions during light clusterization
		// TODO make something smarter than this
		std::map<LightBase*, jmath::xvec4> m_lights;
		std::array<uint32_t, NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z * LIGHTS_PER_CLUSTER> m_clusterLightIndices;
		DynamicCpuBuffer<ClusterEntry, NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z> m_clusterEntryData;
		DynamicCpuBuffer<UINT1, CLUSTER_ITEM_DATA_SIZE> m_clusterItemData;
	};
}
#endif // CLUSTERED_LIGHT_SYSTEM_H
