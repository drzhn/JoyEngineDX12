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
		Skybox();
		void DrawSky(ID3D12GraphicsCommandList* commandList, const ResourceView* colorTextureSrv, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionData) const;
	private:
		ResourceHandle<Texture> m_skyboxTexture;
		ResourceHandle<Mesh> m_skyboxMesh;
		ResourceHandle<GraphicsPipeline> m_skyboxPipeline;
	};
}

#endif // SKYBOX_H
