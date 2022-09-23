﻿#ifndef RAYTRACING_H
#define RAYTRACING_H

#include <d3d12.h>

#include <memory>

#include "CommonEngineStructs.h"
#include "DataBuffer.h"
#include "BufferSorter.h"
#include "ResourceManager/Mesh.h"
#include "ResourceManager/ResourceHandle.h"

namespace JoyEngine
{
	class Raytracing
	{
	public:
		Raytracing();
	private:
		uint32_t m_trianglesLength;
		std::unique_ptr<DataBuffer<uint32_t>> m_keysBuffer;
		std::unique_ptr<DataBuffer<uint32_t>> m_triangleIndexBuffer;
		std::unique_ptr<DataBuffer<Triangle>> m_triangleDataBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_triangleAABBBuffer;
		std::unique_ptr<DataBuffer<AABB>> m_bvhDataBuffer;
		std::unique_ptr<DataBuffer<LeafNode>> m_bvhLeafNodesBuffer;
		std::unique_ptr<DataBuffer<InternalNode>> m_bvhInternalNodesBuffer;

		ResourceHandle<Mesh> m_mesh;
		std::unique_ptr<BufferSorter> m_bufferSorter;
	};
}
#endif // RAYTRACING_H
