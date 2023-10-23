#include "Mesh.h"

#include "JoyAssetHeaders.h"
#include "DataManager/DataManager.h"
#include "EngineDataProvider/EngineDataProvider.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	Mesh::Mesh(const char* path) : Resource(path)
	{
		std::ifstream modelStream = DataManager::Get()->GetFileStream(path, true);

		MeshAssetHeader header = {};
		modelStream.read(reinterpret_cast<char*>(&header), sizeof(MeshAssetHeader));

		InitMesh(
			header.vertexDataSize,
			header.indexDataSize,
			modelStream,
			sizeof(MeshAssetHeader),
			sizeof(MeshAssetHeader) + header.vertexDataSize
		);
		modelStream.close();
	}

	Mesh::Mesh(uint32_t vertexDataSize,
	           uint32_t indexDataSize,
	           std::ifstream& modelStream,
	           uint32_t vertexDataStreamOffset,
	           uint32_t indexDataStreamOffset) : Resource(RandomHash64())
	{
		InitMesh(vertexDataSize,
		         indexDataSize,
		         modelStream,
		         vertexDataStreamOffset,
		         indexDataStreamOffset);
	}

	void Mesh::InitMesh(
		uint32_t vertexDataSize,
		uint32_t indexDataSize,
		std::ifstream& modelStream,
		uint32_t vertexDataStreamOffset,
		uint32_t indexDataStreamOffset)
	{
		m_verticesData = static_cast<Vertex*>(malloc(vertexDataSize));
		m_indicesData = static_cast<Index*>(malloc(indexDataSize));

		//modelStream.clear();
		//modelStream.seekg(vertexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_verticesData), vertexDataSize);

		//modelStream.clear();
		//modelStream.seekg(indexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_indicesData), indexDataSize);

		m_vertexCount = vertexDataSize / sizeof(Vertex);
		m_indexCount = indexDataSize / sizeof(Index);

		MeshContainer* mc = EngineDataProvider::Get()->GetMeshContainer();

		mc->CreateMeshView(vertexDataSize, indexDataSize, m_meshView);

		MemoryManager::Get()->LoadDataToBuffer(m_verticesData, vertexDataSize, mc->GetVertexBuffer(), m_meshView.vertexBufferOffset);
		MemoryManager::Get()->LoadDataToBuffer(m_indicesData, indexDataSize, mc->GetIndexBuffer(), m_meshView.indexBufferOffset);

		m_raytracingGeometryDesc = {
			.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
			.Triangles = {
				.Transform3x4 = 0, // TODO Dont forget to store here transform data. 
				.IndexFormat = DXGI_FORMAT_R32_UINT,
				.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT,
				.IndexCount = m_indexCount,
				.VertexCount = m_vertexCount,
				.IndexBuffer = m_meshView.indexBufferView.BufferLocation,
				.VertexBuffer =
				{
					.StartAddress = m_meshView.vertexBufferView.BufferLocation,
					.StrideInBytes = sizeof(Vertex)
				}
			}
		};
	}

	Mesh::~Mesh()
	{
		free(m_verticesData);
		free(m_indicesData);
	}
}
