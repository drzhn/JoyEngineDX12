#include "Mesh.h"

#include <vector>
#include "JoyAssetHeaders.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		std::ifstream modelStream = DataManager::Get()->GetFileStream(guid, true);

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

	Mesh::Mesh(GUID guid,
	           uint32_t vertexDataSize,
	           uint32_t indexDataSize,
	           std::ifstream& modelStream,
	           uint32_t vertexDataStreamOffset,
	           uint32_t indexDataStreamOffset) : Resource(guid)
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
		m_indicesData = static_cast<uint32_t*>(malloc(indexDataSize));

		modelStream.clear();
		modelStream.seekg(vertexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_verticesData), vertexDataSize);

		modelStream.clear();
		modelStream.seekg(indexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_indicesData), indexDataSize);

		m_vertexCount = vertexDataSize / sizeof(Vertex);
		m_indexCount = indexDataSize / sizeof(uint32_t);

		m_vertexBuffer = std::make_unique<UAVGpuBuffer>(
			m_vertexCount,
			sizeof(Vertex),
			D3D12_RESOURCE_STATE_GENERIC_READ); // for using in raytracing

		m_indexBuffer = std::make_unique<UAVGpuBuffer>(
			m_indexCount,
			sizeof(uint32_t),
			D3D12_RESOURCE_STATE_GENERIC_READ);

		MemoryManager::Get()->LoadDataToBuffer(m_verticesData, vertexDataSize, m_vertexBuffer->GetBuffer());
		MemoryManager::Get()->LoadDataToBuffer(m_indicesData, indexDataSize, m_indexBuffer->GetBuffer());

		m_vertexBufferView = {
			m_vertexBuffer->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress(),
			vertexDataSize,
			sizeof(Vertex)
		};

		m_indexBufferView = {
			m_indexBuffer->GetBuffer()->GetBufferResource()->GetGPUVirtualAddress(),
			indexDataSize,
			DXGI_FORMAT_R32_UINT,
		};
	}

	Mesh::~Mesh()
	{
		free(m_verticesData);
		free(m_indicesData);
	}
}
