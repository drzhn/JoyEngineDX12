#ifndef CLUSTERED_LIGHT_SYSTEM_H
#define CLUSTERED_LIGHT_SYSTEM_H

#include "CommonEngineStructs.h"
#include "ILightSystem.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/DynamicCpuBuffer.h"

namespace JoyEngine
{
	class SharedMaterial;
	class ResourceView;

	class ClusteredLightSystem : public ILightSystem
	{
	public:

		explicit ClusteredLightSystem(uint32_t frameCount);

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

		DirectionalLightData& GetDirectionalLightData() override
		{
			return m_directionalLightData;
		}

		void Update(const uint32_t frameIndex) override;
		uint32_t RegisterDirectionalLight() override;
		void UnregisterDirectionalLight() override;

	private:
		const uint32_t m_frameCount;

		DirectionalLightData m_directionalLightData;
		std::unique_ptr<DynamicCpuBuffer<DirectionalLightData>> m_directionalLightDataBuffer;
		std::unique_ptr<DepthTexture> m_directionalShadowmap;
	};
}
#endif // CLUSTERED_LIGHT_SYSTEM_H
