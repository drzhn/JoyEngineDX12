#ifndef MESH_H
#define MESH_H

#include <memory>
#include <fstream>

#include "Common/Resource.h"
#include "ResourceManager/Buffer.h"
#include "Utils/GUID.h"

namespace JoyEngine
{
	class Mesh final : public Resource
	{
	public:
		Mesh() = delete;

		explicit Mesh(GUID);
		explicit Mesh(GUID, uint32_t, uint32_t, std::ifstream&, uint32_t, uint32_t);

		~Mesh() final;

		[[nodiscard]] uint32_t GetIndexSize() const noexcept { return m_indexCount; }

		[[nodiscard]] uint32_t GetVertexSize() const noexcept { return m_vertexCount; }

		[[nodiscard]] ComPtr<ID3D12Resource> GetVertexBuffer() const noexcept { return m_vertexBuffer->GetBuffer(); }

		[[nodiscard]] ComPtr<ID3D12Resource> GetIndexBuffer() const noexcept { return m_indexBuffer->GetBuffer(); }

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
	};
}


#endif //MESH_H
