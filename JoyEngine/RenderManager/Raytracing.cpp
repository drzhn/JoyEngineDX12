#include "Raytracing.h"

#include "ResourceManager/ResourceManager.h"

namespace JoyEngine
{
	AABB Whole = {
		.min = glm::vec3(-6.f, -6.f, -6.f),
		.max = glm::vec3(6.f, 6.f, 6.f),
	};

	uint32_t ExpandBits(uint32_t v)
	{
		v = (v * 0x00010001u) & 0xFF0000FFu;
		v = (v * 0x00000101u) & 0x0F00F00Fu;
		v = (v * 0x00000011u) & 0xC30C30C3u;
		v = (v * 0x00000005u) & 0x49249249u;
		return v;
	}

	uint32_t Morton3D(float x, float y, float z)
	{
		x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
		y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
		z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
		uint32_t xx = ExpandBits(static_cast<uint32_t>(x));
		uint32_t yy = ExpandBits(static_cast<uint32_t>(y));
		uint32_t zz = ExpandBits(static_cast<uint32_t>(z));
		return xx * 4 + yy * 2 + zz;
	}

	void GetCentroidAndAABB(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3* centroid, AABB* aabb)
	{
		glm::vec3 min = glm::vec3(
			std::min(std::min(a.x, b.x), c.x) - 0.001f,
			std::min(std::min(a.y, b.y), c.y) - 0.001f,
			std::min(std::min(a.z, b.z), c.z) - 0.001f
		);
		glm::vec3 max = glm::vec3(
			std::max(std::max(a.x, b.x), c.x) + 0.001f,
			std::max(std::max(a.y, b.y), c.y) + 0.001f,
			std::max(std::max(a.z, b.z), c.z) + 0.001f
		);

		*centroid = (min + max) * 0.5f;
		*aabb = {
			.min = min,
			.max = max,
		};
	}

	glm::vec3 NormalizeCentroid(glm::vec3 centroid)
	{
		glm::vec3 ret = centroid;
		ret.x -= Whole.min.x;
		ret.y -= Whole.min.y;
		ret.z -= Whole.min.z;
		ret.x /= (Whole.max.x - Whole.min.x);
		ret.y /= (Whole.max.y - Whole.min.y);
		ret.z /= (Whole.max.z - Whole.min.z);
		return ret;
	}

	Raytracing::Raytracing()
	{
		static_assert(sizeof(Triangle) == 128);
		static_assert(sizeof(AABB) == 32);

		const GUID meshGuid = GUID::StringToGuid("47000776-d056-43a2-bacb-c3e54701294a");
		m_mesh = ResourceManager::Get()->LoadResource<Mesh>(meshGuid);

		m_keysBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleIndexBuffer = std::make_unique<DataBuffer<uint32_t>>(DATA_ARRAY_COUNT, MAX_UINT);
		m_triangleDataBuffer = std::make_unique<DataBuffer<Triangle>>(DATA_ARRAY_COUNT);
		m_triangleAABBBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);

		m_bvhDataBuffer = std::make_unique<DataBuffer<AABB>>(DATA_ARRAY_COUNT);
		m_bvhLeafNodesBuffer = std::make_unique<DataBuffer<LeafNode>>(DATA_ARRAY_COUNT);
		m_bvhInternalNodesBuffer = std::make_unique<DataBuffer<InternalNode>>(DATA_ARRAY_COUNT);

		m_trianglesLength = m_mesh->GetIndexCount() / 3;

		Vertex* vertices = m_mesh->GetVertices();
		uint32_t* indices = m_mesh->GetIndices();

		for (uint32_t i = 0; i < m_trianglesLength; i++)
		{
			glm::vec3 a = vertices[indices[i * 3 + 0]].pos;
			glm::vec3 b = vertices[indices[i * 3 + 1]].pos;
			glm::vec3 c = vertices[indices[i * 3 + 2]].pos;

			glm::vec3 centroid;
			AABB aabb = {};

			GetCentroidAndAABB(a, b, c, &centroid, &aabb);
			centroid = NormalizeCentroid(centroid);
			uint32_t mortonCode = Morton3D(centroid.x, centroid.y, centroid.z);
			m_keysBuffer->GetLocalData()[i] = mortonCode;
			m_triangleIndexBuffer->GetLocalData()[i] = i;
			m_triangleDataBuffer->GetLocalData()[i] = Triangle
			{
				.a = a,
				.b = b,
				.c = c,
				.a_uv = vertices[indices[i * 3 + 0]].texCoord,
				.b_uv = vertices[indices[i * 3 + 1]].texCoord,
				.c_uv = vertices[indices[i * 3 + 2]].texCoord,
				.a_normal = vertices[indices[i * 3 + 0]].normal,
				.b_normal = vertices[indices[i * 3 + 1]].normal,
				.c_normal = vertices[indices[i * 3 + 2]].normal,
			};
			m_triangleAABBBuffer->GetLocalData()[i] = aabb;
		}

		m_keysBuffer->UploadCpuData();
		m_triangleIndexBuffer->UploadCpuData();
		m_triangleDataBuffer->UploadCpuData();
		m_triangleAABBBuffer->UploadCpuData();
	}
}
