#include "TransformProvider.h"

#include "SceneManager/WorldManager.h"

namespace JoyEngine
{
	void TransformProvider::Init()
	{
		for (int i = 0; i < m_pool.GetFrameCount(); i++)
			m_pool.Update(i);
	}

	void TransformProvider::Update()
	{
		m_pool.Update(WorldManager::Get()->GetRenderer().GetCurrentFrameIndex());
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
