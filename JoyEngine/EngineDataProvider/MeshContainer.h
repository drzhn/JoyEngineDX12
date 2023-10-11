#ifndef MESH_CONTAINER_H
#define MESH_CONTAINER_H
#include <memory>

#include "Common/Allocators/LinearAllocator.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"

namespace JoyEngine
{
	struct MeshView
	{
		uint64_t offset = 0;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	};

	// Store every single mesh in one single buffer
	class MeshContainer
	{
	public:
		MeshContainer();
		void CreateMeshView(uint64_t vertexBufferSize, uint64_t indexBufferSize, MeshView& outView);
		[[nodiscard]] ResourceView* GetBufferSRV() const noexcept { return m_buffer->GetSRV(); }

	private:
		LinearAllocator m_allocator;
		std::unique_ptr<UAVGpuBuffer> m_buffer;
	};
}
#endif // MESH_CONTAINER_H
