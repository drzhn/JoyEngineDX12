#ifndef MESH_H
#define MESH_H

#include <memory>

#include "CommonEngineStructs.h"
#include "Buffers/UAVGpuBuffer.h"
#include "EngineDataProvider/MeshContainer.h"

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

		[[nodiscard]] D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() noexcept { return &m_meshView.vertexBufferView; }

		[[nodiscard]] D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() noexcept { return &m_meshView.indexBufferView; }

		[[nodiscard]] D3D12_RAYTRACING_GEOMETRY_DESC* GetRaytracingGeometryDescPtr() noexcept { return &m_raytracingGeometryDesc; }

		[[nodiscard]] uint32_t GetVerticesBufferOffsetInBytes() const { return m_meshView.vertexBufferOffset; }
		[[nodiscard]] uint32_t GetIndicesBufferOffsetInBytes() const { return m_meshView.indexBufferOffset; }

		[[nodiscard]] bool IsLoaded() const noexcept override { return true; }

	private:
		void InitMesh(uint32_t, uint32_t, std::ifstream&, uint32_t, uint32_t);

	private:
		uint32_t m_indexCount = 0;
		uint32_t m_vertexCount = 0;

		MeshView m_meshView;

		D3D12_RAYTRACING_GEOMETRY_DESC m_raytracingGeometryDesc;

		Vertex* m_verticesData; // TODO Get rid of storing cpu vertex and index data
		uint32_t* m_indicesData;
	};
}


#endif //MESH_H
