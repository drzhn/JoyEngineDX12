#ifndef BVH_CONSTRUCTOR_H
#define BVH_CONSTRUCTOR_H

#include <cstdint>

#include "CommonEngineStructs.h"
#include "DataBuffer.h"
#include "ResourceManager/ConstantBuffer.h"
#include "ResourceManager/ResourceHandle.h"
#include "ResourceManager/SharedMaterial.h"

namespace JoyEngine
{
	class ComputeDispatcher;

	class BVHConstructor
	{
	public:
		BVHConstructor(
			uint32_t trianglesCount,
			DataBuffer<uint32_t>* sortedMortonCodes,
			DataBuffer<uint32_t>* sortedTriangleIndices,
			DataBuffer<AABB>* triangleAABB,
			DataBuffer<InternalNode>* internalNodes,
			DataBuffer<LeafNode>* leafNodes,
			DataBuffer<AABB>* bvhData, 
			ComputeDispatcher* dispatcher
		);

		void ConstructTree();
		void ConstructBVH();
	private:
		uint32_t m_trianglesCount;
		DataBuffer<uint32_t>* m_sortedMortonCodes;
		DataBuffer<uint32_t>* m_sortedTriangleIndices;
		DataBuffer<AABB>* m_triangleAABB;
		DataBuffer<InternalNode>* m_internalNodes;
		DataBuffer<LeafNode>* m_leafNodes;
		DataBuffer<AABB>* m_bvhData;

		std::unique_ptr<DataBuffer<uint32_t>> m_atomics;

		ResourceHandle<ComputePipeline> m_bvhTreeConstructorPipeline;
		ResourceHandle<ComputePipeline> m_bvhMergerPipeline;

		ConstantBuffer<BVHConstructorData> m_data;

		ComputeDispatcher* m_dispatcher = nullptr;
	};
}
#endif // BVH_CONSTRUCTOR_H
