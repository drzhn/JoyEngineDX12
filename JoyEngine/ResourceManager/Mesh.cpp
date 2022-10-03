#include "Mesh.h"

#include <vector>
#include "CommonEngineStructs.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		std::ifstream modelStream = DataManager::Get()->GetFileStream(guid, true);

		uint32_t vertexDataSize;
		uint32_t indexDataSize;
		modelStream.read(reinterpret_cast<char*>(&vertexDataSize), sizeof(uint32_t));
		modelStream.read(reinterpret_cast<char*>(&indexDataSize), sizeof(uint32_t));

		InitMesh(
			vertexDataSize,
			indexDataSize,
			modelStream,
			sizeof(uint32_t) + sizeof(uint32_t),
			sizeof(uint32_t) + sizeof(uint32_t) + vertexDataSize
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
		m_verticesData = reinterpret_cast<Vertex*>(malloc(vertexDataSize));
		m_indicesData = reinterpret_cast<uint32_t*>(malloc(indexDataSize));

		modelStream.clear();
		modelStream.seekg(vertexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_verticesData), vertexDataSize);

		modelStream.clear();
		modelStream.seekg(indexDataStreamOffset);
		modelStream.read(reinterpret_cast<char*>(m_indicesData), indexDataSize);

		m_vertexCount = vertexDataSize / sizeof(Vertex);
		m_indexCount = indexDataSize / sizeof(uint32_t);

		m_vertexBuffer = std::make_unique<Buffer>(
			vertexDataSize,
			D3D12_RESOURCE_STATE_GENERIC_READ, // TODO change this to more particular mask later
			D3D12_HEAP_TYPE_DEFAULT);

		m_indexBuffer = std::make_unique<Buffer>(
			indexDataSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);

		MemoryManager::Get()->LoadDataToBuffer((void*)m_verticesData, vertexDataSize, m_vertexBuffer.get());
		MemoryManager::Get()->LoadDataToBuffer((void*)m_indicesData, indexDataSize, m_indexBuffer.get());

		m_vertexBufferView = {
			m_vertexBuffer->GetBufferResource()->GetGPUVirtualAddress(),
			vertexDataSize,
			sizeof(Vertex)
		};

		m_indexBufferView = {
			m_indexBuffer->GetBufferResource()->GetGPUVirtualAddress(),
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
