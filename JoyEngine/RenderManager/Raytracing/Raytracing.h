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
	class Raytracing
	{
	public:
		Raytracing();
		void PrepareBVH();
		void DrawGizmo(ID3D12GraphicsCommandList* commandList, MVP mvp);
	private:
		uint32_t m_trianglesLength;
		std::unique_ptr<DataBuffer<uint32_t>> m_keysBuffer;
		std::unique_ptr<DataBuffer<uint32_t>> m_triangleIndexBuffer;
		std::unique_ptr<DataBuffer<Triangle>> m_triangleDataBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_triangleAABBBuffer;

		std::unique_ptr<DataBuffer<AABB>> m_bvhDataBuffer;
		std::unique_ptr<DataBuffer<LeafNode>> m_bvhLeafNodesBuffer;
		std::unique_ptr<DataBuffer<InternalNode>> m_bvhInternalNodesBuffer;

		ConstantCpuBuffer<BVHConstructorData> m_bvhConstructionData;

		ResourceHandle<Mesh> m_mesh;
		std::unique_ptr<BufferSorter> m_bufferSorter;
		std::unique_ptr<BVHConstructor> m_bvhConstructor;
		std::unique_ptr<ComputeDispatcher> m_dispatcher;

		ResourceHandle<SharedMaterial> m_gizmoAABBDrawerSharedMaterial;
	};
}
#endif // RAYTRACING_H
