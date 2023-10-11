#ifndef MESH_CONTAINER_H
#define MESH_CONTAINER_H
#include <memory>

#include "Common/Allocators/LinearAllocator.h"
#include "ResourceManager/Buffers/UAVGpuBuffer.h"

namespace JoyEngine
{
	struct MeshView
	{
		uint32_t vertexBufferOffset = 0;
		uint32_t indexBufferOffset = 0;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	};

	// Store every single mesh in one single buffer
	class MeshContainer
	{
	public:
		MeshContainer();
		void CreateMeshView(uint64_t vertexBufferSize, uint64_t indexBufferSize, MeshView& outView);

		[[nodiscard]] Buffer* GetVertexBuffer() const { return m_vertexBuffer->GetBuffer(); }
		[[nodiscard]] Buffer* GetIndexBuffer() const { return m_indexBuffer->GetBuffer(); }

		[[nodiscard]] ResourceView* GetVertexBufferSRV() const noexcept { return m_vertexBuffer->GetSRV(); }
		[[nodiscard]] ResourceView* GetIndexBufferSRV() const noexcept { return m_indexBuffer->GetSRV(); }

	private:
		LinearAllocator m_vertexAllocator;
		std::unique_ptr<UAVGpuBuffer> m_vertexBuffer;

		LinearAllocator m_indexAllocator;
		std::unique_ptr<UAVGpuBuffer> m_indexBuffer;
	};
}
#endif // MESH_CONTAINER_H
