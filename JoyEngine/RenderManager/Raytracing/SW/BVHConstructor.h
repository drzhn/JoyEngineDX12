#ifndef BVH_CONSTRUCTOR_H
#define BVH_CONSTRUCTOR_H

#include <cstdint>

#include "CommonEngineStructs.h"
#include "RenderManager/Raytracing/DataBuffer.h"
#include "ResourceManager/Buffers/ConstantCpuBuffer.h"
#include "ResourceManager/Pipelines/ComputePipeline.h"

namespace JoyEngine
{
	class ComputeDispatcher;

	class BVHConstructor
	{
	public:
		BVHConstructor(
			DataBuffer<uint32_t>* sortedMortonCodes,
			DataBuffer<uint32_t>* sortedTriangleIndices,
			DataBuffer<AABB>* triangleAABB,
			DataBuffer<InternalNode>* internalNodes,
			DataBuffer<LeafNode>* leafNodes,
			DataBuffer<AABB>* bvhData,
			ComputeDispatcher* dispatcher,
			ConstantCpuBuffer<BVHConstructorData>* bvhConstructionData
		);

		void ConstructTree();
		void ConstructBVH() const;
	private:
		DataBuffer<uint32_t>* m_sortedMortonCodes;
		DataBuffer<uint32_t>* m_sortedTriangleIndices;
		DataBuffer<AABB>* m_triangleAABB;
		DataBuffer<InternalNode>* m_internalNodes;
		DataBuffer<LeafNode>* m_leafNodes;
		DataBuffer<AABB>* m_bvhData;
		ConstantCpuBuffer<BVHConstructorData>* m_bvhConstructionData;

		std::unique_ptr<DataBuffer<uint32_t>> m_atomics;

		std::unique_ptr<ComputePipeline> m_bvhTreeConstructorPipeline;
		std::unique_ptr<ComputePipeline> m_bvhMergerPipeline;


		ComputeDispatcher* m_dispatcher = nullptr;
	};
}
#endif // BVH_CONSTRUCTOR_H
