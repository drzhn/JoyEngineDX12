#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <d3d12.h>

#include <memory>

#include "CommonEngineStructs.h"
#include "DataBuffer.h"
#include "BufferSorter.h"
#include "BVHConstructor.h"
#include "RenderManager/ComputeDispatcher.h"
#include "ResourceManager/Mesh.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class IRenderManager;

	class Raytracing
	{
	public:
		Raytracing() = delete;
		Raytracing(DXGI_FORMAT mainColorFormat, DXGI_FORMAT swapchainFormat, uint32_t width, uint32_t height);
		void PrepareBVH();
		void ProcessRaytracing(ID3D12GraphicsCommandList* commandList, ResourceView* engineDataResourceView);
		void DebugDrawRaytracedImage(ID3D12GraphicsCommandList* commandList);
		void DrawGizmo(ID3D12GraphicsCommandList* commandList, const ViewProjectionMatrixData* viewProjectionMatrixData) const;
	private:
		DXGI_FORMAT m_mainColorFormat;
		DXGI_FORMAT m_swapchainFormat;
		uint32_t m_width;
		uint32_t m_height;

		uint32_t m_trianglesLength;

		std::unique_ptr<UAVTexture> m_raytracedTexture;

		std::unique_ptr<DataBuffer<uint32_t>> m_keysBuffer;
		std::unique_ptr<DataBuffer<uint32_t>> m_triangleIndexBuffer;
		std::unique_ptr<DataBuffer<Triangle>> m_triangleDataBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_triangleAABBBuffer;

		std::unique_ptr<DataBuffer<AABB>> m_bvhDataBuffer;
		std::unique_ptr<DataBuffer<LeafNode>> m_bvhLeafNodesBuffer;
		std::unique_ptr<DataBuffer<InternalNode>> m_bvhInternalNodesBuffer;

		ConstantCpuBuffer<BVHConstructorData> m_bvhConstructionData;

		ResourceHandle<Mesh> m_mesh;
		ResourceHandle<Texture> m_texture;
		std::unique_ptr<BufferSorter> m_bufferSorter;
		std::unique_ptr<BVHConstructor> m_bvhConstructor;
		std::unique_ptr<ComputeDispatcher> m_dispatcher;

		ResourceHandle<ComputePipeline> m_raytracingPipeline;
		ResourceHandle<GraphicsPipeline> m_raytracedImageApplier;

		ResourceHandle<GraphicsPipeline> m_gizmoAABBDrawerGraphicsPipeline;
		ResourceHandle<GraphicsPipeline> m_debugRaytracingTextureDrawGraphicsPipeline;
	};
}
#endif // RAYTRACING_H
