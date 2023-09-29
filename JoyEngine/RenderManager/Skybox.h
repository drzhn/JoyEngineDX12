#ifndef SKYBOX_H
#define SKYBOX_H

#include "ResourceManager/Mesh.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/Texture.h"
#include "ResourceManager/Buffers/ConstantCpuBuffer.h"
#include "ResourceManager/Pipelines/GraphicsPipeline.h"

namespace JoyEngine
{
	class ResourceView;

	class Skybox
	{
	public:
		Skybox();
		void DrawSky(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionData) const;
		[[nodiscard]] ResourceView* GetSkyboxTextureDataSrv() const { return m_skyboxTextureIndexData.GetView(); }

	private:
		ResourceHandle<Texture> m_skyboxTexture;
		ResourceHandle<Mesh> m_skyboxMesh;
		std::unique_ptr<GraphicsPipeline> m_skyboxPipeline;

		ConstantCpuBuffer<TextureIndexData> m_skyboxTextureIndexData;
	};
}

#endif // SKYBOX_H
