#include "TransformProvider.h"

namespace JoyEngine
{
	void TransformProvider::Update(uint32_t frameIndex)
	{
		ObjectMatricesData* data = m_pool.GetDynamicBuffer().GetPtr(frameIndex);

		for (uint32_t i =0; i < m_pool.GetDataArray().size(); i++)
		{
			if (!m_pool.GetUsedItems()[i]) continue;

			data->data[i] = m_pool.GetElem(i).GetModelMatrix();
		}

	}

	uint32_t TransformProvider::Allocate()
	{
		return m_pool.Allocate();
	}

	void TransformProvider::Free(const uint32_t index)
	{
		m_pool.Free(index);
	}

	Transform& TransformProvider::GetTransform(const uint32_t transformIndex)
	{
		return m_pool.GetElem(transformIndex);
	}

	ResourceView* TransformProvider::GetObjectMatricesBufferView(uint32_t frameIndex)
	{
		return m_pool.GetDynamicBuffer().GetView(frameIndex);
	}
}
