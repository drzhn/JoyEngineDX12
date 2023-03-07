#ifndef MODEL_DATA_SYSTEM_H
#define MODEL_DATA_SYSTEM_H

#include <glm/fwd.hpp>

#include "CommonEngineStructs.h"
#include "ResourceManager/Buffers/DynamicBufferPool.h"

namespace JoyEngine
{
	class ModelDataSystem
	{
	public:

	private:
		DynamicBufferPool<glm::mat4, ObjectMatricesData, OBJECT_SIZE> m_pool;
	};
}
#endif // MODEL_DATA_SYSTEM_H
