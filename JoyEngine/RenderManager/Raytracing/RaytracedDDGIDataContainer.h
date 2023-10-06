#ifndef RAYTRACED_DDGI_DATA_CONTAINER_H
#define RAYTRACED_DDGI_DATA_CONTAINER_H

#include "CommonEngineStructs.h"
#include "DataBuffer.h"
#include "RenderManager/ComputeDispatcher.h"
#include "ResourceManager/Mesh.h"
#include "ResourceManager/SharedMaterial.h"
#include "ResourceManager/Buffers/DynamicCpuBuffer.h"

namespace JoyEngine
{
	inline RaytracedProbesData g_raytracedProbesData = {
		.gridMin = glm::vec3(-27, 1, -12),
		.cellSize = 4.1f,
		.gridX = 14,
		.gridY = 8,
		.gridZ = 6,
		.useDDGI = 1,
		.skyboxTextureIndex = 0,
	};

	// common structs, buffers, shaders for both software and hardware DDGI systems
	class RaytracedDDGIDataContainer
	{
	public:
		RaytracedDDGIDataContainer() = delete;
		RaytracedDDGIDataContainer(
			std::set<SharedMaterial*>& sceneSharedMaterials,
			uint32_t frameCount,
			DXGI_FORMAT mainColorFormat,
			DXGI_FORMAT depthFormat);
		void SetFrameData(uint32_t frameIndex, const ResourceView* skyboxTextureIndexDataView);
		void UploadSceneData() const;

		[[nodiscard]] ComputeDispatcher* GetDispatcher() const { return m_dispatcher.get(); }
		[[nodiscard]] const std::set<SharedMaterial*>& GetSceneSharedMaterials() const { return m_sceneSharedMaterials; }
		[[nodiscard]] ResourceView* GetProbesDataView(uint32_t frameIndex) const { return m_raytracedProbesData.GetView(frameIndex); }
		[[nodiscard]] ResourceView* GetTrianglesDataView() const { return m_triangleDataBuffer->GetSRV(); }
		[[nodiscard]] ResourceView* GetMeshDataView() const { return m_meshDataBuffer->GetSRV(); }
		[[nodiscard]] Mesh* GetDebugSphereMesh() const { return m_debugSphereProbeMesh.Get(); }

		void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList, const ResourceView* texture) const;
		void DebugDrawProbes(ID3D12GraphicsCommandList* commandList, uint32_t frameIndex, const ViewProjectionMatrixData* viewProjectionMatrixData, const ResourceView* probeIrradianceTexture) const;

	private:
		const std::set<SharedMaterial*>& m_sceneSharedMaterials;
		std::unique_ptr<DataBuffer<TrianglePayload>> m_triangleDataBuffer;
		std::unique_ptr<DataBuffer<MeshData>> m_meshDataBuffer;

		DynamicCpuBuffer<RaytracedProbesData> m_raytracedProbesData;

		std::unique_ptr<ComputeDispatcher> m_dispatcher;

		ResourceHandle<Mesh> m_debugSphereProbeMesh;
		std::unique_ptr<GraphicsPipeline> m_debugDrawProbesGraphicsPipeline;
		std::unique_ptr<GraphicsPipeline> m_debugRaytracingTextureDrawGraphicsPipeline;
	};
}
#endif // RAYTRACED_DDGI_DATA_CONTAINER_H
