#include "MeshContainer.h"

#define MESH_CONTAINER_SIZE (64*1024*1024)

namespace JoyEngine
{
	MeshContainer::MeshContainer():
		m_allocator(MESH_CONTAINER_SIZE, 1)
	{
		m_buffer = std::make_unique<UAVGpuBuffer>(
			MESH_CONTAINER_SIZE,
			1,
			D3D12_RESOURCE_STATE_GENERIC_READ); // for using in raytracing
	}

	void MeshContainer::CreateMeshView(uint64_t vertexBufferSize, uint64_t indexBufferSize, MeshView& outView)
	{

	}
}
