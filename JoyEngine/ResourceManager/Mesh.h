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

		~Mesh() final;

		[[nodiscard]] size_t GetIndexSize() const noexcept { return m_indexSize; }

		[[nodiscard]] size_t GetVertexSize() const noexcept { return m_vertexSize; }

		[[nodiscard]] ComPtr<ID3D12Resource> GetVertexBuffer() const noexcept { return m_vertexBuffer->GetBuffer(); }

		[[nodiscard]] ComPtr<ID3D12Resource> GetIndexBuffer() const noexcept { return m_indexBuffer->GetBuffer(); }

		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView()  noexcept { return &m_vertexBufferView; }

		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView()  noexcept { return &m_indexBufferView; }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }


	private:
		size_t m_indexSize;
		size_t m_vertexSize;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;


		std::ifstream m_modelStream;
	};
}


#endif //MESH_H
