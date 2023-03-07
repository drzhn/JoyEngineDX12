#ifndef MESH_H
#define MESH_H

#include <memory>
#include <wrl/client.h>

#include "CommonEngineStructs.h"
#include "Buffers/Buffer.h"

#include "Common/Resource.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Mesh final : public Resource
	{
	public:
		Mesh() = delete;

		explicit Mesh(GUID);
		explicit Mesh(GUID, uint32_t, uint32_t, std::ifstream&, uint32_t, uint32_t);

		~Mesh() override;

		[[nodiscard]] uint32_t GetIndexCount() const noexcept { return m_indexCount; }

		[[nodiscard]] uint32_t GetVertexCount() const noexcept { return m_vertexCount; }

		[[nodiscard]] Vertex* GetVertices() const noexcept { return m_verticesData; }

		[[nodiscard]] uint32_t* GetIndices() const noexcept { return m_indicesData; }

		[[nodiscard]] ComPtr<ID3D12Resource> GetVertexBuffer() const noexcept { return m_vertexBuffer->GetBufferResource(); }

		[[nodiscard]] ComPtr<ID3D12Resource> GetIndexBuffer() const noexcept { return m_indexBuffer->GetBufferResource(); }

		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() noexcept { return &m_vertexBufferView; }

		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() noexcept { return &m_indexBufferView; }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void InitMesh(uint32_t, uint32_t, std::ifstream&, uint32_t, uint32_t);

	private:
		uint32_t m_indexCount = 0;
		uint32_t m_vertexCount = 0;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

		Vertex* m_verticesData;
		uint32_t* m_indicesData;
	};
}


#endif //MESH_H
