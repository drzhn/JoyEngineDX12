#include "WorldMatricesProvider.h"

namespace JoyEngine
{
	void WorldMatricesProvider::Update()
	{

	}

	uint32_t WorldMatricesProvider::Allocate()
	{
		uint32_t index = -1;
		m_pool.Allocate(index);
		return index;
	}
}
