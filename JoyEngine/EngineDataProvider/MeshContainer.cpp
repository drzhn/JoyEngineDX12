#include "MeshContainer.h"

#include "CommonEngineStructs.h"

#define TRIANGLE_LIMIT (512*1024)
#define MESH_CONTAINER_ALIGNMENT 1

namespace JoyEngine
{
	MeshContainer::MeshContainer():
		m_vertexAllocator(TRIANGLE_LIMIT * sizeof(Vertex) * 3, MESH_CONTAINER_ALIGNMENT),
		m_indexAllocator(TRIANGLE_LIMIT * sizeof(Index) * 3, MESH_CONTAINER_ALIGNMENT)
	{
		m_vertexBuffer = std::make_unique<UAVGpuBuffer>(
			TRIANGLE_LIMIT * 3,
			sizeof(Vertex),
			D3D12_RESOURCE_STATE_GENERIC_READ);

		m_indexBuffer = std::make_unique<UAVGpuBuffer>(
			TRIANGLE_LIMIT * 3,
			sizeof(Index),
			D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void MeshContainer::CreateMeshView(uint64_t vertexBufferSize, uint64_t indexBufferSize, MeshView& outView)
	{
		const uint32_t vertexBufferOffset = static_cast<uint32_t>(m_vertexAllocator.Allocate(vertexBufferSize));
		const uint32_t indexBufferOffset = static_cast<uint32_t>(m_indexAllocator.Allocate(indexBufferSize));

		outView = {
			.vertexBufferOffset = vertexBufferOffset,
			.indexBufferOffset = indexBufferOffset,
			.vertexBufferView = {
				.BufferLocation = GetVertexBuffer()->GetBufferResource()->GetGPUVirtualAddress() + vertexBufferOffset,
				.SizeInBytes = static_cast<uint32_t>(vertexBufferSize),
				.StrideInBytes = sizeof(Vertex)
			},
			.indexBufferView = {
				.BufferLocation = GetIndexBuffer()->GetBufferResource()->GetGPUVirtualAddress() + indexBufferOffset,
				.SizeInBytes = static_cast<uint32_t>(indexBufferSize),
				.Format = DXGI_FORMAT_R32_UINT
			}
		};
	}
}
