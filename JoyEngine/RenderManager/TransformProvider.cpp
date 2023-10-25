#include "TransformProvider.h"

namespace JoyEngine
{
	void TransformProvider::Update(const uint32_t frameIndex)
	{
		m_pool.Update(frameIndex);
	}

	uint32_t TransformProvider::Allocate()
	{
		return m_pool.Allocate();
	}

	void TransformProvider::Free(const uint32_t index)
	{
		m_pool.Free(index);
	}

	jmath::mat4x4& TransformProvider::GetMatrix(const uint32_t transformIndex)
	{
		return m_pool.GetValue(transformIndex);
	}

	ResourceView* TransformProvider::GetObjectMatricesBufferView(uint32_t frameIndex)
	{
		return m_pool.GetDynamicBuffer().GetView(frameIndex);
	}
}
