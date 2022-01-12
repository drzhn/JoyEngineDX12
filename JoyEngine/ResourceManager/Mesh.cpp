#include "Mesh.h"

#include "JoyContext.h"

#include <vector>
#include "RenderManager/JoyTypes.h"
#include "DataManager/DataManager.h"
#include "MemoryManager/MemoryManager.h"

namespace JoyEngine
{
	Mesh::Mesh(GUID guid) : Resource(guid)
	{
		m_modelStream = JoyContext::Data->GetFileStream(guid, true);

		uint32_t verticesDataSize;
		uint32_t indicesDataSize;
		m_modelStream.read(reinterpret_cast<char*>(&verticesDataSize), sizeof(uint32_t));
		m_modelStream.read(reinterpret_cast<char*>(&indicesDataSize), sizeof(uint32_t));

		m_vertexSize = verticesDataSize / sizeof(Vertex);
		m_indexSize = indicesDataSize / sizeof(uint32_t);

		m_vertexBuffer = std::make_unique<Buffer>(
			verticesDataSize,
			D3D12_RESOURCE_STATE_GENERIC_READ, // TODO change this to more particular mask later
			D3D12_HEAP_TYPE_DEFAULT);

		m_indexBuffer = std::make_unique<Buffer>(
			indicesDataSize,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_HEAP_TYPE_DEFAULT);

		m_vertexBuffer->LoadData(m_modelStream, sizeof(uint32_t) + sizeof(uint32_t));

		m_indexBuffer->LoadData(m_modelStream, sizeof(uint32_t) + sizeof(uint32_t) + verticesDataSize);
	}

	Mesh::~Mesh()
	{
		//JoyContext::Memory->DestroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
		//JoyContext::Memory->DestroyBuffer(m_indexBuffer, m_indexBufferMemory);
	}
}
