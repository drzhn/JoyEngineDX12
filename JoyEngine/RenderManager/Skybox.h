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
		[[nodiscard]] ResourceView* GetSkyboxTextureSrv() const { return m_skyboxTexture->GetSRV(); }

	private:
		ResourceHandle<Texture> m_skyboxTexture;
		ResourceHandle<Mesh> m_skyboxMesh;
		std::unique_ptr<GraphicsPipeline> m_skyboxPipeline;
	};
}

#endif // SKYBOX_H
