#ifndef SKYBOX_H
#define SKYBOX_H

#include "ResourceManager/Mesh.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/Texture.h"

namespace JoyEngine
{
	class ResourceView;

	class Skybox
	{
	public:
		Skybox() = delete;
		explicit Skybox(ResourceView* colorTextureSRV);
		void DrawSky(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionData) const;
	private:
		ResourceView* m_colorTextureSRV;
		ResourceHandle<Texture> m_skyboxTexture;
		ResourceHandle<Mesh> m_skyboxMesh;
		ResourceHandle<GraphicsPipeline> m_skyboxPipeline;
	};
}

#endif // SKYBOX_H
